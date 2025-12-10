#pragma once
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include "lib/nholmann/json.hpp"
#include <thread>
#include "quoteprocess.hpp"
#include "agentconnexionprocess.hpp"
#include "exchangeProcess.hpp"



class Agent {
private:
  int agent_index;
  std::map<std::string, int> wallet;           //[cle: entreprise, valeur: nombre d'actions]
  double cash;                                 //l'argent que possede l'agent
  unsigned int number_agents;
  std::map<std::string, std::vector<double>> server_sent_quotes;   //les cotations envoye par le serveur
  std::mutex to_lock;
  bool trading_begin;
  QuoteProcess quote;                          //pour connecter un agent avec le market
  AgentConnexionProcess connexion_with_agent;  //pour connecter l'agent avec les autres agents
  std::vector<std::thread> agent_threads;      //pour lancer les threads de conexion avec les autres clients
  exchangeProcess exchange;
  public:
  /* j'instancie quoteprocess et agentconnexionprocess directement dans le constructeur de agent */
  Agent(int market_port, int index_agent,int port_local, unsigned int number_agent ) : quote(market_port), connexion_with_agent(port_local,*(this)),
  exchange(*this),agent_index(index_agent),to_lock(),number_agents(number_agent),trading_begin(false) {}

  /* getter */
  exchangeProcess& getExchange()  { return exchange; }
  int getAgentIndex() const { return agent_index; }
  void setTradeBegin (bool value) {trading_begin = value;}
  bool getTradeBegin () { return trading_begin;}
  std::map<std::string, int>& getWallet() { return wallet; }
  double& getCash()  { return cash; }
  unsigned int getNumberAgents() const { return number_agents; }
  std::map<std::string, std::vector<double>>& getServerSentQuotes() { return server_sent_quotes; }
  std::mutex& getMutex() { return to_lock; }

  /* lecture du portfolio de l'agent */
  void read_from_json(std::string path);

  /* naitre le thread pour se connecter au serveur */
  std::thread spawnThreadToMarket() { return std::thread([this] { runConnexionToMarket(); }); }
  /* naitre le thread pour se connecter aux agents */
  std::vector<std::thread> spawnThreadToAgent() {return runConnexionWithAgent();}

  private:

  void runConnexionToMarket();
  std::vector<std::thread> runConnexionWithAgent();

};