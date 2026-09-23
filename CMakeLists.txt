cmake_minimum_required(VERSION 3.16)
project(dice-party LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(dice-party
  src/main.cpp
  src/io/terminal.cpp
)

target_include_directories(dice-party PRIVATE src)

enable_testing()

add_executable(dice-tests
  tests/test_dice.cpp
)

target_include_directories(dice-tests PRIVATE src)

add_test(NAME dice-tests COMMAND dice-tests)

add_executable(scoring-tests
  tests/test_scoring.cpp
)

target_include_directories(scoring-tests PRIVATE src)

add_test(NAME scoring-tests COMMAND scoring-tests)
