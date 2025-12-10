#include "quoteprocess.hpp"


void QuoteProcess::connectToMarket(std::map<std::string, int> quote ,std::map<std::string, std::vector<double>>& server_sent_quotes,std::mutex& m,bool& trading_begin){
       try{
              m.lock();
              std::clog << "[agent] Connexion to Market ...." << std::endl;
              m.unlock();

              std::this_thread::sleep_for(std::chrono::seconds(5));

              /* le role de static_cast<in_port_t> est de convertir int en in_port_t pour eviter:
              'narrowing conversion warning'
               */

              sockpp::tcp_connector conn({ip_market, static_cast<in_port_t>(market_port)});
              if (!conn) {
                     std::lock_guard<std::mutex> lock(m);
                     throw std::runtime_error("connexion refused to the market");
              }

              m.lock();
              std::clog << "[agent] Market connection established" << std::endl;
              m.unlock();

              auto socket = std::make_shared<sockpp::tcp_connector>(std::move(conn));

              /* envoie du portefeuille */
              auto sock = sendQuote(quote,std::move(socket),m);

              /* recevoir les cotations */
              receiveCotationsFromMarket(std::move(sock), server_sent_quotes, trading_begin);


       }catch(std::exception& e){
             std::cout << e.what() << std::endl;
       }
}

void QuoteProcess::receiveCotationsFromMarket(std::shared_ptr<sockpp::tcp_connector> socket,
                                              std::map<std::string, std::vector<double>>& server_sent_quotes,bool& trading_begin) {
       try {
              while (true) {
                     uint32_t taille = 0;
                     auto taille_res = socket->read(&taille, sizeof(taille));

                     if (!taille_res || taille_res.value() != sizeof(taille)) {
                            throw std::runtime_error("Erreur lors de la lecture de la taille du JSON");
                     }

                     std::string json(taille, '\0');

                     auto res = socket->read(json.data(), taille);
                     if (!res || res.value() != taille) {
                            throw std::runtime_error("Read error or no data received.");
                     }

                     /* parser le json */
                     nlohmann::json data;
                     try {
                            data = nlohmann::json::parse(json);
                     } catch (const std::exception& e) {
                            std::cerr << "[Agent] Erreur JSON parsing " << e.what() << std::endl;
                            return;
                     }

                     // Remplir map
                     for (const auto& pair : data.items()) {
                            if (!pair.value().is_array()) {
                                   std::cerr << "[Agent] Erreur Json: " << pair.key() << " n'est pas un tableau de doubles\n";
                                   continue;
                            }

                            try {
                                   std::vector<double> quotes = pair.value().get<std::vector<double>>();
                                   server_sent_quotes[pair.key()].insert(
                                       server_sent_quotes[pair.key()].end(),
                                       quotes.begin(),
                                       quotes.end()
                                   );
                            } catch (const std::exception& e) {
                                   std::cerr << "[Agent] Erreur conversion pour " << pair.key() << " : " << e.what() << std::endl;
                            }
                     }

                   /*  std::clog << "[Agent] Contenu de server_sent_quotes après mise à jour :" << std::endl;
                     for (const auto& [entreprise, valeurs] : server_sent_quotes) {
                            std::clog << " - " << entreprise << " : ";
                            for (const double& val : valeurs) {
                                   std::clog << val << " ";
                            }
                            std::clog << std::endl;
                     }*/

                     std::clog << "[Agent] Quotes received by the market : " << server_sent_quotes.size() << " company." << std::endl;

                     /* informer le market que la negociation a commence
                     if (trading_begin) {
                            std::string msg = "start-trade";
                            socket->write(msg.data(), msg.size());
                     }*/

              }
              }catch (const std::exception& e) {
                     std::cerr << "[Erreur réception] " << e.what() << std::endl;
              }

}

std::shared_ptr<sockpp::tcp_connector> QuoteProcess::sendQuote(std::map<std::string,int> quote, std::shared_ptr<sockpp::tcp_connector> conn,std::mutex& m) {

       std::string company;
       m.lock();

       std::clog << "[Agent] Sending quote to market ...." << std::endl;


       for (auto const &[tokens, shares] : quote) {
              company += tokens + " ;";
       }

       /*envoie des entreprises ou l'agent possede des actions */
       conn->write(company.data(), company.size());

       /*un log pour voir le format de message qu'on envoie */
     /*  std::clog << "apres la boucle for:" << std::endl; */
       std::clog << "[Agent] portfolio " << company << std::endl;

       std::this_thread::sleep_for(std::chrono::seconds(7));
       std::clog << "[Agent] operation accomplished " << std::endl;
       m.unlock();

       return conn;
}