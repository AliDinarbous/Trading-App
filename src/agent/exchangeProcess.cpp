//
// Created by MEDI on 13/05/2025.
//
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "exchangeProcess.hpp"
#include "agent.hpp"
/*
 * PROTOCOL DE VENTE/ACHAT:
 * AGENT ANALYSE LES COTATIONS RECUS, IL METS DANS UN VECTOR LES ACTIONS A VENDRE.
 * IL ENVOIE SES OFFRES AU DEUXIEME AGENTS
 * RECOIT CE QUE L'AUTRE AGENT LUI PROPOSE D'ACHETER
 * CHACUN ENVOIE UN MESSAGE POUR INDIQUER SI IL ACCEPTE LE DEAL OU NON
 * SI OUI ON MODIFIE LE PORTFEUILLE
 * SI AUCUNE OFFRE ON AFFICHE AUCUNE OFFRE A PROPOSER
  */
void exchangeProcess::lancerTrading(std::shared_ptr<sockpp::tcp_socket> socket) {

    std::this_thread::sleep_for(std::chrono::seconds(180));
    std::clog << "[Exchange] Start of trading" << std::endl;

    auto start = std::chrono::steady_clock::now();
    while (true) {
        std::vector<Offer> offres_possibles;

        {
            std::lock_guard<std::mutex> lock(agent.getMutex());

         /*   std::clog << "[DEBUG] Début de l'analyse des offres possibles...\n"; */
            for (const auto& [entreprise, nombre_actions] : agent.getWallet()) {
            /*    std::clog << "[DEBUG] Entreprise : " << entreprise
                          << ", Actions possedees : " << nombre_actions << std::endl; */

                const auto& cotations = agent.getServerSentQuotes()[entreprise];
             /*   std::clog << "[DEBUG] Nombre de cotations reçues : " << cotations.size() << std::endl; */

                if (cotations.size() < 7) {
              /*      std::clog << "[DEBUG] Pas assez de cotations pour " << entreprise << ", besoin de 7.\n"; */
                    continue;
                }

                if (nombre_actions == 0) {
               /*     std::clog << "[DEBUG] Aucun stock pour " << entreprise << "\n"; */
                    continue;
                }

                double ancienne = cotations[cotations.size() - 7];
                double recente = cotations.back();

                std::clog << "[To Make Decision] Comparison of old = " << ancienne
                          << ", new = " << recente << " -> Threshold = " << (ancienne * 0.95) << std::endl;

                if (recente < ancienne * 0.95) {
                    int quantite_vente = static_cast<int>(nombre_actions * 0.1);
                   /* std::clog << "[DEBUG] Calcul de quantité à vendre : " << quantite_vente << std::endl; */

                    if (quantite_vente == 0) {
                   /*     std::clog << "[DEBUG] Quantité de vente trop faible pour " << entreprise << "\n"; */
                        continue;
                    }

                    double prix_vente = recente * 0.8;
                    double revenu = prix_vente * quantite_vente;

                    offres_possibles.push_back({entreprise, quantite_vente, revenu});
                /*    std::clog << "[DEBUG] Offre ajoutée : " << entreprise << ", qté = " << quantite_vente
                              << ", prix = " << prix_vente << ", revenu = " << revenu << std::endl; */
                } else {
                /*    std::clog << "[DEBUG] Condition de baisse non remplie pour " << entreprise << "\n"; */
                }
            }
        }

        if (!offres_possibles.empty()) {
            auto meilleur = std::max_element(offres_possibles.begin(), offres_possibles.end(),
                [](const Offer& a, const Offer& b) { return a.prix < b.prix; });

            double derniere_valeur = agent.getServerSentQuotes()[meilleur->entreprise].back();
            double prix_unitaire = derniere_valeur * 0.8;

            std::string msg = "offer;" + meilleur->entreprise + ";" + std::to_string(meilleur->quantite) + ";" + std::to_string(prix_unitaire);
            socket->write(msg.data(), msg.size());
            std::clog << "[Exchange] Offre send : " << msg << std::endl;
        } else {
            std::string msg = "no-offer";
            socket->write(msg.data(), msg.size());
            std::clog << "[Exchange] No offer to send." << std::endl;
        }

        // Lecture d'une offre de l'autre agent
        char buf[1024];
        auto res = socket->read(buf, sizeof(buf));
        if (!res || res.value() == 0) continue;

        std::string message(buf, res.value());

        if (message.find("offer;") == 0) {
            size_t pos1 = message.find(';');
            size_t pos2 = message.find(';', pos1 + 1);
            size_t pos3 = message.find(';', pos2 + 1);

            if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
                std::string entreprise = message.substr(pos1 + 1, pos2 - pos1 - 1);
                int quantite = std::stoi(message.substr(pos2 + 1, pos3 - pos2 - 1));
                double prix = std::stod(message.substr(pos3 + 1));

                {
                    std::lock_guard<std::mutex> lock(agent.getMutex());
                    if (agent.getCash() >= prix * quantite) {
                        std::string msg = "sale;" + entreprise + ";" + std::to_string(quantite) + ";" + std::to_string(prix);
                        socket->write(msg.data(), msg.size());

                        agent.getCash() -= prix * quantite;
                        agent.getWallet()[entreprise] += quantite;

                        std::clog << "[Exchange] PURCHASE completed: "
                                  << quantite << " quotes of " << entreprise
                                  << " à " << prix << " euro (total " << prix * quantite << "euro)." << std::endl;
                    } else {
                        std::string msg = "no-sale";
                        socket->write(msg.data(), msg.size());
                        std::clog << "[Exchange] Purchase DENIED: insufficient funds to "
                                  << quantite << " quotes of " << entreprise
                                  << " à " << prix << " euro" << std::endl;
                    }
                }
            }
        } else if (message.find("sale;") == 0) {
            size_t pos1 = message.find(';');
            size_t pos2 = message.find(';', pos1 + 1);
            size_t pos3 = message.find(';', pos2 + 1);

            if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
                std::string entreprise = message.substr(pos1 + 1, pos2 - pos1 - 1);
                int quantite = std::stoi(message.substr(pos2 + 1, pos3 - pos2 - 1));
                double prix = std::stod(message.substr(pos3 + 1));

                std::lock_guard<std::mutex> lock(agent.getMutex());
                agent.getCash() += prix * quantite;
                agent.getWallet()[entreprise] -= quantite;

                std::clog << "[Exchange] SALE completed:  "
                          << quantite << "  shares of  " << entreprise
                          << " at " << prix << " euros (total " << prix * quantite << "euros)." << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - start).count();
        if (elapsed > 1) {
            std::clog << "[Exchange] Time elapsed (1 min). Trading ended." << std::endl;
            break;
        }
    }
}



void exchangeProcess::sendToMarket(const std::string& message) {
    // Pour l'instant on affiche juste les messages
    std::clog << message << std::endl;
}
