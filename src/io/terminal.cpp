#include "io/terminal.hpp"

#include <iostream>

namespace dice_party {

void print_startup(const GameState& game) {
  std::cout << "Dice-Party\n";
  std::cout << "Phase 1 foundation stub. Scoring and play are not implemented.\n";
  std::cout << "Turn " << game.turn << "/" << kTurnCount
            << ", rolls used " << game.rolls_used << "/" << kMaxRollsPerTurn
            << ", unused categories " << unused_category_count(game.scorecard)
            << ".\n";
}

}  // namespace dice_party
