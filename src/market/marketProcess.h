//
// Created by MEDI on 05/05/2025.
//

#ifndef MARKETPROCESS_H
#define MARKETPROCESS_H
#include <fstream>
#include <sockpp/tcp_connector.h>
#include "sockpp/tcp_acceptor.h"
#include "lib/nholmann/json.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

class marketProcess {

public:
    void initConnexion(int port,int numberAgents,    std::map<std::string, std::vector<double>> cotations);
    private:
    /* methode qui cree le json qui contient les cotations a envoyer aux agents */
    nlohmann::json createJsonToSendCotation(const std::string& jsonFilePath,  std::map<std::string, std::vector<double>> cotations);
};



#endif //MARKETPROCESS_H
