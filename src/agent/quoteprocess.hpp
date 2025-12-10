#include<string>
#include<thread>
#include <stdexcept>
#include <iostream>
#include <map>
#include <sockpp/tcp_connector.h>
#include "lib/nholmann/json.hpp"
#include <memory>
#include <vector>
#include <mutex>

class QuoteProcess {
  private:
    std::string ip_market ;                            //adresse ip du serveur
    int market_port ;                                  //port du serveur

  public:

    QuoteProcess(int market_port, std::string ip_market= "127.0.0.1"):market_port(market_port),
    ip_market(ip_market){};                                                          //constructeur

    void connectToMarket(std::map<std::string, int> quote
      ,std::map<std::string, std::vector<double>>& server_sent_quotes, std::mutex& m, bool& trading_begin);                         //la mathode qui contient le code du thread

  private:
    std::shared_ptr<sockpp::tcp_connector> sendQuote(std::map<std::string,int> quote,                                      //la methode va envoyer le porte feuille au serveur
    std::shared_ptr<sockpp::tcp_connector> conn,std::mutex& m);

    void receiveCotationsFromMarket(std::shared_ptr<sockpp::tcp_connector> socket,       //la methode pour lire les cotattions envoyes par le serveur
    std::map<std::string, std::vector<double>> & server_sent_quotes, bool& trading_begin);



};
