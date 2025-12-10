include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/lib/ ${CMAKE_SOURCE_DIR}/src/lib/sockpp/include)

set(AGENT_HPP agent.hpp)

set(AGENT_CPP agent.cpp)
set(AGENTQ_HPP quoteprocess.hpp)

set(AGENTQ_CPP quoteprocess.cpp
        agentconnexionprocess.hpp
        agentconnexionprocess.cpp
        exchangeProcess.cpp)
add_executable(agent main.cpp ${AGENT_HPP} ${AGENT_CPP} ${AGENTQ_HPP} ${AGENTQ_CPP})

target_link_libraries(agent Threads::Threads Sockpp::sockpp)