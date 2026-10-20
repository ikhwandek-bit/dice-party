#pragma once

#include <iosfwd>
#include <random>

#include "game/game.hpp"

namespace dice_party {

// --- Phase 6: terminal UI (SPEC §4.1, §10) ---

// Runs one full game in a terminal. Reads one command per line from `in`,
// writes the board, prompts, and messages to `out`, and rolls dice with
// `gen`. Returns the last game state: final if the game completed, or the
// state reached on quit/EOF.
GameState run_session(std::istream& in, std::ostream& out, std::mt19937& gen);

// Display helpers: pure functions of the state (no reading, no rules).
void print_help(std::ostream& out);
void print_frame(std::ostream& out, const GameState& game);
void print_scoring_tab(std::ostream& out, const GameState& game, bool forced);
void print_final_breakdown(std::ostream& out, const GameState& game);
void print_error(std::ostream& out, const GameState& game);
const char* category_name(Category category);

}  // namespace dice_party
