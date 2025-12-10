#include "agent.hpp"

void Agent::runConnexionToMarket() {

  /* se connecter au marche */
  quote.connectToMarket(wallet,server_sent_quotes,to_lock,trading_begin);

}

std::vector<std::thread> Agent::runConnexionWithAgent() {
  /* se connecter au autre client */
  for (int i = 1; i <= number_agents; ++i) {
    /* on ne se connecte pas a soi meme */
    if (i == agent_index) continue;

    int port_autre_agent = 4000 + i;

    if (i < agent_index) {
      /* je suis client vers les agents ayant un index plus petit */
      to_lock.lock();
      std::clog << "[Agent client for agent"<<i <<"] Connexion to agent ..." << std::endl;
      to_lock.unlock();
      agent_threads.emplace_back([this, i, port_autre_agent]() {
                connexion_with_agent.initConnexionWithAgent(agent_index, to_lock, port_autre_agent);
            });
      }
  }

  /* je serais serveur pour les agents ayant un index plus grand */
  agent_threads.emplace_back([this] {
    connexion_with_agent.receiveAgentConnexion(to_lock, agent_index);
  });

    return std::move(agent_threads);
}

void Agent::read_from_json(std::string path) {

  std::ifstream jsonFile(path); //ouvrir le fichier

  if (!jsonFile.is_open()) { //getion d'erreur au cas ficher ne s'ouvre pas
    std::cerr << "Error opening file " <<std::endl;
    throw std::runtime_error("Error opening file " + path);
  }

  nlohmann::json data = nlohmann::json::parse(jsonFile); //lecture du fichier grace a nlohman

  /* remplissage de std::map et cash */
  for (const auto& pair : data["portfolio"]) {
    std::string company = pair[0];   // cle
    int shares = pair[1];            // valeur
    wallet[company] = shares;
  }
  cash = data["cash"];

  std::clog << "[agent] content of the Json file after download" <<std::endl;
  /* affichage pour s'assurer que tout va bien */
  for (const auto& [company, shares] : wallet) {
    std::cout << "wallet[\"" << company << "\"] = " << shares << std::endl;
  }

  std::cout << "cash = " << cash << ";" << std::endl;

}


