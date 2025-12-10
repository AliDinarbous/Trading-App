#pragma once

#include <thread>
#include <map>
#include <vector>
#include "marketProcess.h"

class Market {
public:

  std::thread spawn() { return std::thread([this] { run(); }); }
  bool loadCotations(const std::string &filePath);

  Market(int port , int numberAgents){
    this->port =port;
    this->numberAgents =numberAgents;
  }

private:
  marketProcess marketprocess;  /* classe pour se connecter aux agents */
  std::map<std::string, std::vector<double>> cotations;  /*lecture du CSV */
  int port;  /* port du serveur */
  int numberAgents;    /* nombre d'agents clients du serveur */
  void run();
};


