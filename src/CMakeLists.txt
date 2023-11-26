cmake_minimum_required(VERSION 3.26)
project(server-thread)

set(CMAKE_C_STANDARD 11)

add_executable(server
        server/server.c
        main.c
        logger/logger.c)