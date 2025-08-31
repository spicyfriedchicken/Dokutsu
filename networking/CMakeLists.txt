cmake_minimum_required(VERSION 3.20)
project(asio_play LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(asio_play main.cpp)
target_include_directories(asio_play PRIVATE /opt/homebrew/include)

# Warnings (clang++)
target_compile_options(asio_play PRIVATE -Wall -Wextra -Wpedantic)

# If you use Asio SSL later:
# find_package(OpenSSL REQUIRED)
# target_link_libraries(asio_play PRIVATE OpenSSL::SSL OpenSSL::Crypto)
