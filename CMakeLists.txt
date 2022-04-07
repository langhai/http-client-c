MATH(EXPR stack_size "512") # 16 Mb
cmake_minimum_required(VERSION 3.21)
project(http_client_c C)

set(CMAKE_C_STANDARD 11)


include_directories(src)

add_executable(http_client_c
        src/base64.h
        src/http-client-c.h
        src/stringx.h
        src/urlparser.h
        test/main.c)
