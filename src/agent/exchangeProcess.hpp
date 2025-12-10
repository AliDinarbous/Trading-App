
#pragma once
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <iostream>
#include <sockpp/tcp_connector.h>
#include "sockpp/tcp_acceptor.h"
#include <memory>
class Agent;

struct Offer {
    std::string entreprise;
    int quantite;
    double prix;
};

class exchangeProcess {
    Agent& agent;
    std::vector<Offer> received_offers;
    std::vector<Offer> received_sale_offers;
public:
    exchangeProcess(Agent& agent) : agent(agent) {}
    void lancerTrading(std::shared_ptr<sockpp::tcp_socket> socket);
    void sendToMarket(const std::string& message);
};


