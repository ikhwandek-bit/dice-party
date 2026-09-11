#include "game/game.hpp"
#include "io/terminal.hpp"

int main() {
  const dice_party::GameState game = dice_party::make_new_game();
  dice_party::print_startup(game);
  return 0;
}
