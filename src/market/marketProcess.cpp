// Created by MEDI on 05/05/2025.

#include "marketProcess.h"
#include <sockpp/tcp_connector.h>
#include "sockpp/tcp_acceptor.h"
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <thread>

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r");
    auto end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

/* extraire les cotations du portfeuille */
std::map<std::string, std::vector<double>> processPortefeuille(
    const std::string& portefeuille,
    const std::map<std::string, std::vector<double>>& cotations) {

    std::stringstream ss(portefeuille);
    std::string action;
    std::map<std::string, std::vector<double>> result;

    while (std::getline(ss, action, ';')) {
        action = trim(action); // supprime les espaces et retours à la ligne
        if (action.empty()) continue; // ignore les entrées vides

        auto it = cotations.find(action);
        if (it != cotations.end() && !it->second.empty()) {
            result[action] = it->second;
        } else {
            std::cout << "Attention : action inconnue ou sans données : " << action << std::endl;
        }
    }

    return result;
}


nlohmann::json marketProcess::createJsonToSendCotation(const std::string& jsonFilePath,  std::map<std::string, std::vector<double>> cotations) {
    try {
        static int index_cotation = 0;  // index global pour la creation des fichiers
        nlohmann::json j; //creation du json

     /*   std::clog << "declarer json" << std::endl;
        std::cout << "Répertoire courant : " << std::filesystem::current_path() << std::endl;*/

        if (cotations.empty()) {
            std::cerr << "[Server] ERREUR : cotations à exporter est vide." << std::endl;
        }

        /* remplir le json */
        for (const auto& pair : cotations) {
            const auto& symbol = pair.first;
            const auto& valeurs = pair.second;

            if (index_cotation < valeurs.size()) {
                j[symbol] = { valeurs[index_cotation] };
            } else {
                j[symbol] = {};
            }
        }

        /* ouvrir le json */
        std::ofstream file(jsonFilePath);

        if (!file.is_open()) {
            throw std::runtime_error("Erreur lors de l'ouverture du fichier JSON");
        }

        file << j.dump(4);
        file.close();

   /*    std::clog << "[Server] Fichier JSON créé à l’index " << index_cotation << std::endl; */

        ++index_cotation;
        return j;

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return nlohmann::json();
    }
    return nlohmann::json();

}


void marketProcess::initConnexion(int port, int numberAgents, std::map<std::string, std::vector<double>> cotations) {
    try {
        sockpp::tcp_acceptor acc(port);

        if (!acc) {
            throw std::runtime_error("Unable to jdsdjb to server");
        }

        std::clog << "[market] server ready on port" << port << ", waiting " << numberAgents << " Agents..." << std::endl;

        int connectedAgents = 0;

        /* ce vector servira a stocker les sockets de tout les agents pour l'envoie des cotations */
        std::vector<sockpp::tcp_socket> clients;
        clients.reserve(numberAgents);

        /*ce vector servira a stocker les cotations de chaque agent */
        std::vector<std::map<std::string, std::vector<double>>> cotationsFiltres;

        while (connectedAgents != numberAgents) {
            auto res = acc.accept();
            if (!res) {
                throw std::runtime_error("Erreur de connexion");
                continue;
            }
            sockpp::tcp_socket sock = std::move(res.release());

            std::cout << "[Server] connected agents (" << (connectedAgents + 1) << "/" << numberAgents << ")" << std::endl;

            /* lecture du portfeuille de l'agent connecte */
            char buf[1024];
            auto readRes = sock.read(buf, sizeof(buf));
            if (!readRes) {
                std::cerr << "Erreur : aucune donnée reçue." << std::endl;
                continue;
            }
            size_t n = readRes.value();
            std::string portefeuille(buf, n);
            std::map<std::string, std::vector<double>> cotesAgent = processPortefeuille(portefeuille, cotations);
            cotationsFiltres.push_back(cotesAgent);

            /* enregistrer le socket de cette agent */
            clients.push_back(std::move(sock));
            ++connectedAgents;

        }


        /*envoie des cotations */

        std::clog <<"[Server] sending quotes to all the agents " << std::endl;
       /* std::cout << "Répertoire courant : " << std::filesystem::current_path() << std::endl;*/

        /*bool trading_started = false;*/


        int nombre_envoie = 0;
        auto start = std::chrono::steady_clock::now();
        int max_seconds = 180;

        while (true) {
            /* envoie des cotations aux clients  */
            for (int j = 0; j < clients.size(); ++j) {

                auto& client = clients[j];
                const auto& co = cotationsFiltres[j];
                std::string fileName = "data/market/cotations/quotes" + std::to_string(nombre_envoie) + "_agent_" + std::to_string(j + 1) + ".json";

                nlohmann::json cotations_json = createJsonToSendCotation(fileName, co);
                std::string json_a_envoyer = cotations_json.dump(3);

                /* envoie de la taille pour eviter des ..., et du json */
                uint32_t taille = json_a_envoyer.size();
                client.write(&taille, sizeof(taille));  // envoyer la taille (4 octets)
                client.write(json_a_envoyer.data(), taille);
            }
            ++nombre_envoie;
            std::clog <<"[Server] Mission Accomplished, The server resend the quotes after two minutes" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));

            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            if (elapsed >= max_seconds) {
                std::clog << "[Market] Timeout reached after 6 HOURS. Stopping quotes." << std::endl;
                break;  // sort du for
            }

        }
    }catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }



}


