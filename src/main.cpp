#include <iostream>
#include <random>

#include "io/terminal.hpp"

int main() {
  std::mt19937 gen(std::random_device{}());
  dice_party::run_session(std::cin, std::cout, gen);
  return 0;
}
