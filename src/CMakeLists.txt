cmake_minimum_required(VERSION 3.26)
project(server-thread LANGUAGES C)

set(CMAKE_C_STANDARD 11)



add_executable(
        ${CMAKE_PROJECT_NAME}
        server/thread_pool.c
        main.c
        logger/logger.c
        server/socket_poll.c
        server/server.c
        server/request.c)

add_executable(
        client
        client.c)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC logger)


