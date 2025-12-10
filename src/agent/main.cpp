#include "argparse/argparse.hpp"
#include "sockpp/socket.h"
#include "agent.hpp"
#include <fstream>
#include <iostream>
#include "lib/nholmann/json.hpp"
#include <filesystem>

int main(int argc, char *argv[]) {
 /* je parse les arguments necessaire pour le lancement d'un agent */
  argparse::ArgumentParser program("agent");

  program.add_argument("-p")
    .help("Port on which the market process is listening (eg. 4000).")
    .scan<'i', int>();

  program.add_argument("-n")
    .help("Total number of agents expected in the simulation")
    .scan<'i', int>();

  program.add_argument("-i")
    .help("Index de cet agent")
    .scan<'i', int>();

  program.add_argument("-m")
    .help("Port on which the agent will accept incoming connections")
    .scan<'i', int>();

  program.add_argument("-d")
    .help("Path for the json file")
    .default_value(std::string(""));

  /* gestion d'erreur au cas ou une erreur lors du parsing */
  try {
    program.parse_args(argc, argv);
  }
  catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return -1;
  }

  /* recuperation des arguments */
  auto agent_port = program.get<int>("-p");
  auto agent_number = program.get<int>("-n");
  auto agent_index = program.get<int>("-i");
  auto port_market = program.get<int>("-m");
  auto path = program.get<std::string>("-d");

  /* j'initialise la biblio sockup */
  sockpp::initialize();

  /* petit test pour voir si tout fonctionne bien
  std::cout << agent_port << std::endl;
  std::cout << agent_number << std::endl;
  std::cout << agent_index << std::endl;
  std::cout << port_market << std::endl;
  std::cout << path << std::endl; */

  /*s'assurer que le repertoire du fichier passer en arguments exist */
  std::filesystem::directory_entry entry{path};
  try {
    if (entry.exists()) {
      throw std::runtime_error("File does not exist");
    }
  }catch (std::exception &err) {
     std::cerr << err.what() << std::endl;
  }

  /* instanciation d'un agent */
  Agent agent(port_market,agent_index,agent_port,agent_number);

  /*lecture du json dans la std::map*/
  agent.read_from_json(path);

  /* lancement du thread vers le market */
  auto th_market = agent.spawnThreadToMarket();
  /* lancement du thread vers les agents */
  auto th_client  = agent.spawnThreadToAgent();

  th_market.join();
  for (auto& th : th_client) {
    th.join();
  }



  return 0;
}