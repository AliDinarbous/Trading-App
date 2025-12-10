#include "market.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <sockpp/tcp_connector.h>
#include "sockpp/tcp_acceptor.h"



void Market::run() {
   marketprocess.initConnexion(port,numberAgents,cotations);  //methode qui implement le serveur
}

bool Market::loadCotations(const std::string &filePath) {
 /* lecture du fichier csv */
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier : " << filePath << std::endl;
        return false;
    }

    std::map<std::string, std::vector<double>> cotations;
    std::string line;
    std::vector<std::string> actions;

    if (std::getline(file, line)) {
        std::istringstream headerStream(line);
        std::string action;
        while (std::getline(headerStream, action, ',')) {
            if (!action.empty()) {
                actions.push_back(action);
                cotations[action] = {};
            }
        }
    }
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string value;
        size_t idx = 0;

        std::getline(lineStream, value, ',');

        while (std::getline(lineStream, value, ',')) {
            if (!value.empty()) {
                cotations[actions[idx]].push_back(std::stod(value));
            } else if (!cotations[actions[idx]].empty()) {
                cotations[actions[idx]].push_back(cotations[actions[idx]].back());
            }
            ++idx;
        }
    }

    for (auto it = cotations.begin(); it != cotations.end();) {
        if (it->second.empty()) {
           /* std::cout << "Action ignored : " << it->first << std::endl;*/
            it = cotations.erase(it);  // Effacer l'élément
        } else {
            ++it;
        }
    }

    this->cotations = cotations;

    std::clog <<"[Market] content of the CSV file after download" << std::endl;
    for (const auto& p : cotations) {
        std::cout << "company: " << p.first << " - quotes: ";
            int nb = 0;

        for (const double& val : p.second) {
                  if (++nb == 10) break;

            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    return true;
}


