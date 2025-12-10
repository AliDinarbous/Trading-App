#pragma once
#include <string>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <map>
#include <sockpp/tcp_connector.h>
#include "sockpp/tcp_acceptor.h"
#include <memory>
#include <vector>
#include <mutex>
#include <sstream>


class Agent;
class AgentConnexionProcess {
    private:
    Agent& agent;
    int local_port;           //port d'ecoute
    public:
    AgentConnexionProcess(int port,Agent& agent):local_port(port),agent(agent){};                 //constructeur
    void receiveAgentConnexion(std::mutex& m,int index_agent);                      //agent en tant que serveur pour agent
    void initConnexionWithAgent (int index_agent,std::mutex& m, int agent_port);    //agent en tant que client pour agent
};