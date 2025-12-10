#include "agentconnexionprocess.hpp"
#include "agent.hpp"

void AgentConnexionProcess::receiveAgentConnexion(std::mutex& m,int index_agent) {

    try {

        sockpp::tcp_acceptor acc(local_port);
        if (!acc) {
            throw std::runtime_error("Agent failed to accept connexion ");
        }

        m.lock();
        std::clog << "[Agent] server ready in port: " << local_port <<  std::endl;
        m.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(5));

        while (true) {
            auto res = acc.accept();
            if (!res) {
                std::ostringstream oss;
                oss << "[Agent as Server] An agent failed to connect";
                std::lock_guard<std::mutex> lock(m);
                throw std::runtime_error(oss.str());
            }

            {
                std::lock_guard<std::mutex> lock(m);
                std::clog << "[Agent as Server] Connected with an agent" << std::endl;
            }


            auto socket = std::make_shared<sockpp::tcp_socket>(res.release());

            std::thread t([this, socket]() {
                agent.getExchange().lancerTrading(socket);
            });
            t.detach();
        }
    }catch (std::exception& err) {

        std::cerr << err.what() << std::endl;
    };

}

void AgentConnexionProcess::initConnexionWithAgent (int index_agent,std::mutex& m, int agent_port) {
    try{

        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::string Ip_agent = "127.0.0.1";

        sockpp::tcp_connector conn({Ip_agent, static_cast<in_port_t>(agent_port)});
        if (!conn) {
            std::lock_guard<std::mutex> lock(m);
            throw std::runtime_error("connexion to agent refused");
        }

        m.lock();
        std::clog << "[Agent]  connection established with the agent" << std::endl;
        m.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(10));

        /*lancer le thread de vente */
        auto raw_sock = conn.release();
        auto socket = std::shared_ptr<sockpp::tcp_socket>(new sockpp::tcp_socket(std::move(raw_sock)));

        std::thread t([this, socket]() {
        agent.getExchange().lancerTrading(socket);
        });
        t.join();

    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
}