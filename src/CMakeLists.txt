cmake_minimum_required(VERSION 3.23)
project(server-thread LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)


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


