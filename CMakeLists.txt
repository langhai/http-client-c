cmake_minimum_required(VERSION 3.21)
project(http_client_c C)

set(CMAKE_C_STANDARD 11)
add_compile_options(-fms-extensions)

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/encoding)

#file(GLOB SOURCES
#        ${SOURCES}/src/encoding/*
#        )

add_executable(http_client_c
        ${CMAKE_SOURCE_DIR}/test/main.c
    )

target_link_libraries(http_client_c m)