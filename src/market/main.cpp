#include "argparse/argparse.hpp"
#include "sockpp/socket.h"
#include <iostream>
#include <string>
#include "market.hpp"
#include <filesystem>

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("market");
    sockpp::initialize();

    program.add_argument("-p")
        .help("port")
        .default_value(8080)
        .scan<'i', int>();

    program.add_argument("-n")
        .help("numberAgents")
        .default_value(1)
        .scan<'i', int>();

    program.add_argument("-f")
        .help("pathFile")
        .default_value(std::string("")) ;

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return -1;
    }

    auto inputPort = program.get<int>("-p");
    auto inputNumberAgents = program.get<int>("-n");
    auto inputPathFile = program.get<std::string>("-f");

  /*  std::cout << "Port: " << inputPort << std::endl;
    std::cout << "Number of Agents: " << inputNumberAgents << std::endl;
    std::cout << "Path File: " << inputPathFile << std::endl;
    std::cout << std::endl; */


    Market market(inputPort, inputNumberAgents);

    if (!market.loadCotations(inputPathFile)) {
        std::cerr << "Err" << std::endl;
        return -1;
    }

    /* creer un repertoire ou les json qui contiennt les cotations vont etre stocket */
    std::string repertory = "data/market/cotations"; // ou un chemin absolu
    try {
        if (!std::filesystem::exists(repertory)) {
            std::filesystem::create_directories(repertory);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
    }


    auto th = market.spawn();
    th.join();

    return 0;
}
