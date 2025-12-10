cmake_minimum_required(VERSION 3.24)
project(sae_market_agent)

set(CMAKE_CXX_STANDARD 17)

option(SOCKPP_BUILD_SHARED "Build shared library" ON)
option(SOCKPP_BUILD_STATIC "Build static library" OFF)
option(SOCKPP_WITH_OPENSSL "TLS Secure Sockets with OpenSSL" OFF)
option(SOCKPP_WITH_MBEDTLS "TLS Secure Sockets with Mbed TLS" OFF)

if(NOT WIN32)
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        option(SOCKPP_WITH_CAN "Include support for Linux SocketCAN components" OFF)
    endif()
    option(SOCKPP_WITH_UNIX_SOCKETS "Include support for UNIX-domain sockets" ON)
else()
    option(SOCKPP_WITH_UNIX_SOCKETS "Include support for UNIX-domain sockets" OFF)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_subdirectory(src)
