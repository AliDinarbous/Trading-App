include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/lib/ ${CMAKE_SOURCE_DIR}/src/lib/sockpp/include)

set(MARKET_HPP market.hpp)

set(MARKET_CPP market.cpp)

set(MARKETP_H marketProcess.h)

set(MARKETP_CPP marketProcess.cpp)
add_executable(market main.cpp ${MARKET_HPP} ${MARKET_CPP} ${MARKETP_H} ${MARKETP_CPP})

target_link_libraries(market Threads::Threads Sockpp::sockpp)