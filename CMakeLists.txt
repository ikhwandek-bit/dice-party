cmake_minimum_required(VERSION 3.16)
project(dice-party LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(dice-party
  src/main.cpp
  src/io/terminal.cpp
)

target_include_directories(dice-party PRIVATE src)
