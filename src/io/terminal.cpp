#include "io/terminal.hpp"

#include <cstddef>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

namespace dice_party {

const char* category_name(Category category) {
  switch (category) {
    case Category::Ones: return "Ones";
    case Category::Twos: return "Twos";
    case Category::Threes: return "Threes";
    case Category::Fours: return "Fours";
    case Category::Fives: return "Fives";
    case Category::Sixes: return "Sixes";
    case Category::ThreeOfAKind: return "Three of a Kind";
    case Category::FourOfAKind: return "Four of a Kind";
    case Category::FullHouse: return "Full House";
    case Category::SmallStraight: return "Small Straight";
    case Category::LargeStraight: return "Large Straight";
    case Category::FiveOfAKind: return "Five of a Kind";
    case Category::Chance: return "Chance";
  }
  return "?";
}

void print_help(std::ostream& out) {
  out << "Dice-Party: five dice, 13 turns, one unused category per turn.\n"
      << "Dice tab keys: R = roll | 1-5 = lock/unlock a die | S = scoring tab | Q = quit\n"
      << "Scoring tab keys: 0-12 = category (see card), S = back to dice.\n";
}

namespace {

std::string trim(const std::string& text) {
  const std::size_t first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  const std::size_t last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1);
}

std::string pad_left(std::string text, std::size_t width) {
  if (text.size() < width) text.insert(0, width - text.size(), ' ');
  return text;
}

std::string pad_right(std::string text, std::size_t width) {
  if (text.size() < width) text.resize(width, ' ');
  return text;
}

std::string face_text(const Die& die) {
  if (die.face >= kMinFace && die.face <= kMaxFace) return std::to_string(die.face);
  return "-";
}

// Whole-line integer parse: rejects non-numeric input and trailing junk.
bool parse_int(const std::string& text, int& value) {
  std::istringstream stream(text);
  stream >> value;
  if (!stream) return false;
  std::string rest;
  stream >> rest;
  return rest.empty();
}

void print_status(std::ostream& out, const GameState& game) {
  const int rolls_left = kMaxRollsPerTurn - game.rolls_used;
  out << "Turn " << game.turn << "/" << kTurnCount
      << " | rolls used " << game.rolls_used << "/" << kMaxRollsPerTurn
      << " (" << rolls_left << " left)\n";
}

void print_dice(std::ostream& out, const GameState& game) {
  out << "Die    :";
  for (int i = 0; i < kDieCount; ++i) out << pad_left(std::to_string(i + 1), 3);
  out << "\nFace   :";
  for (int i = 0; i < kDieCount; ++i) out << pad_left(face_text(game.dice[i]), 3);
  out << "\nLocks  :";
  for (int i = 0; i < kDieCount; ++i) out << pad_left(game.dice[i].locked ? "L" : ".", 3);
  out << "   (L = locked, . = rerolls)\n";
}

void print_scorecard(std::ostream& out, const GameState& game) {
  for (int i = 0; i < kCategoryCount; ++i) {
    const Category category = static_cast<Category>(i);
    const CategorySlot& slot = game.scorecard.slots[i];
    out << ' ' << pad_left(std::to_string(i), 2) << ' '
        << pad_right(category_name(category), 16);
    if (slot.filled) {
      out << pad_left(std::to_string(slot.score), 3) << "  used\n";
    } else {
      out << "   -  unused\n";
    }
  }
}

void print_totals(std::ostream& out, const GameState& game) {
  const Scorecard& card = game.scorecard;
  out << "Upper subtotal : " << upper_total(card) << '\n';
  if (upper_bonus_earned(card)) {
    out << "Upper bonus    : " << kUpperBonusPoints << " (earned)\n";
  } else {
    out << "Upper bonus    : 0 (35 if Upper subtotal reaches " << kUpperBonusThreshold << ")\n";
  }
  out << "Lower subtotal : " << lower_total(card) << '\n';
  if (game.five_of_a_kind_bonus_total > 0) {
    out << "5-kind bonus   : +" << game.five_of_a_kind_bonus_total << '\n';
  }
  out << "Grand total    : " << total_score(game) << '\n';
}

}  // namespace

void print_frame(std::ostream& out, const GameState& game) {
  out << '\n' << std::string(44, '-') << '\n';
  print_status(out, game);
  print_dice(out, game);
  print_scorecard(out, game);
  print_totals(out, game);
}

void print_scoring_tab(std::ostream& out, const GameState& game, bool forced) {
  const bool joker = is_five_of_a_kind_bonus(game);
  const int joker_face = joker ? current_five_face(game) : 0;
  for (int i = 0; i < kCategoryCount; ++i) {
    const Category category = static_cast<Category>(i);
    const CategorySlot& slot = game.scorecard.slots[i];
    out << ' ' << pad_left(std::to_string(i), 2) << ' '
        << pad_right(category_name(category), 16);
    if (slot.filled) {
      out << pad_left(std::to_string(slot.score), 3) << "  used";
    } else if (game.rolls_used == 0) {
      out << "   -  (roll first)";
    } else if (joker && !is_joker_choice_legal(game, category)) {
      out << "   -  (not allowed now)";
    } else {
      const int would_score = joker ? joker_score_for(category, joker_face)
                                    : score_for_category(game, category);
      out << pad_left(std::to_string(would_score), 3) << "  (if chosen)";
    }
    out << '\n';
  }
  if (forced) {
    out << "You must score now: choose a category (0-12).\n";
  } else {
    out << "Enter 0-12 to score, or S to go back.\n";
  }
}

void print_error(std::ostream& out, const GameState& game) {
  switch (game.illegal_move.type) {
    case IllegalMoveType::None:
      break;
    case IllegalMoveType::AllDiceLocked:
      out << "Cannot roll: all five dice are locked. Unlock a die first (press its number).\n";
      break;
    case IllegalMoveType::MaxRollsReached:
      out << "Cannot roll: all " << kMaxRollsPerTurn
          << " rolls are used this turn. Choose a category.\n";
      break;
    case IllegalMoveType::MaxTurnsReached:
      out << "Cannot start another turn: the 13-turn game is already over.\n";
      break;
    case IllegalMoveType::LockBeforeFirstRoll:
      out << "Cannot lock or unlock dice before the first roll of the turn.\n";
      break;
    case IllegalMoveType::InvalidDieIndex:
      out << "Invalid die: use 1-5.\n";
      break;
    case IllegalMoveType::NoRollYet:
      out << "You must roll before choosing a category.\n";
      break;
    case IllegalMoveType::CategoryAlreadyFilled:
      out << "That category is already used. Choose an unused one.\n";
      break;
    case IllegalMoveType::InvalidCategory:
      out << "Invalid category: use 0-12.\n";
      break;
    case IllegalMoveType::GameAlreadyOver:
      out << "The game is already over.\n";
      break;
    case IllegalMoveType::BonusForcedStop:
      out << "Cannot roll: this five of a kind earned the +100 bonus; choose a category now.\n";
      break;
    case IllegalMoveType::JokerForcedCategory: {
      const int face = current_five_face(game);
      const Category matching = joker_matching_upper(face);
      if (!game.scorecard.slots[static_cast<int>(matching)].filled) {
        out << "Joker rules: you must take " << category_name(matching) << " for " << 5 * face
            << ".\n";
      } else if (lower_joker_has_open(game.scorecard)) {
        out << "Joker rules: choose an unused Lower category (not Five of a Kind).\n";
      } else {
        out << "Joker rules: the Lower section is full; choose an unused Upper category for 0.\n";
      }
      break;
    }
  }
}

void print_final_breakdown(std::ostream& out, const GameState& game) {
  out << "\n=== Game over: all 13 categories filled ===\n\n";
  print_scorecard(out, game);
  out << '\n';
  print_totals(out, game);
  out << "\nFinal total: " << total_score(game) << '\n';
}

namespace {

// One visit to the scoring tab. Keeps prompting until the player records a
// category, goes back (only when not forced), or input ends.
// Returns false only when the session must abort (input ended).
bool scoring_loop(std::istream& in, std::ostream& out, GameState& game, bool forced) {
  for (;;) {
    out << "\n--- Scoring tab ---\n";
    print_scoring_tab(out, game, forced);
    out << (forced ? "[0-12 = score] > " : "[0-12 = score | S = back] > ");
    std::string line;
    if (!std::getline(in, line)) {
      out << "\nInput ended: game aborted, no final score.\n";
      return false;
    }
    const std::string command = trim(line);
    if (command.empty()) continue;
    if (command == "S" || command == "s") {
      if (forced) {
        out << "You must choose a category now: enter 0-12.\n";
        continue;
      }
      return true;
    }
    if (command == "R" || command == "r") {
      if (is_five_of_a_kind_bonus(game)) {
        out << "Cannot roll: this five of a kind earned the +100 bonus; choose a category now.\n";
      } else if (forced) {
        out << "Cannot roll: all " << kMaxRollsPerTurn
            << " rolls are used this turn. Enter 0-12.\n";
      } else {
        out << "Rolling happens in the dice tab: press S first, then R.\n";
      }
      continue;
    }
    int key = 0;
    if (!parse_int(command, key)) {
      out << "Invalid input \"" << command << "\": enter 0-12"
          << (forced ? "." : " or S to go back.") << '\n';
      continue;
    }
    const Category category = static_cast<Category>(key);
    const bool bonus_turn = is_five_of_a_kind_bonus(game);
    GameState next = select_category(game, category);
    if (next.illegal_move.type != IllegalMoveType::None) {
      if (next.illegal_move.type == IllegalMoveType::CategoryAlreadyFilled) {
        out << "Category already used: " << category_name(category)
            << ". Choose a different one.\n";
      } else {
        print_error(out, next);
      }
      continue;
    }
    game = next;
    out << "Recorded " << game.scorecard.slots[static_cast<int>(category)].score << " in "
        << category_name(category) << ".\n";
    if (bonus_turn) out << "Five-of-a-Kind Bonus: +100!\n";
    return true;
  }
}

}  // namespace

GameState run_session(std::istream& in, std::ostream& out, std::mt19937& gen) {
  GameState game = make_new_game();
  print_help(out);
  bool playing = true;
  while (playing && !is_game_over(game)) {
    print_frame(out, game);
    if (must_select_category(game)) {
      // SPEC §5.5/§11: after roll 3 (or a bonus five of a kind) the turn can
      // only end through a category choice.
      if (is_five_of_a_kind_bonus(game)) {
        out << "\nFIVE OF A KIND with Five of a Kind already at 50: +100 bonus, Joker rules "
               "apply.\n"
            << "No more rolls this turn: choose a category now.\n";
      } else {
        out << "\nNo rolls left: choose a category now.\n";
      }
      if (!scoring_loop(in, out, game, /*forced=*/true)) playing = false;
      continue;
    }
    out << "\n[R = roll | 1-5 = lock/unlock | S = score | Q = quit] > ";
    std::string line;
    if (!std::getline(in, line)) {
      out << "\nInput ended: game aborted, no final score.\n";
      break;
    }
    const std::string command = trim(line);
    if (command.empty()) continue;
    if (command == "R" || command == "r") {
      game = roll_dice(game, gen);
      if (game.illegal_move.type != IllegalMoveType::None) print_error(out, game);
    } else if (command == "S" || command == "s") {
      if (!scoring_loop(in, out, game, /*forced=*/false)) playing = false;
    } else if (command == "Q" || command == "q") {
      out << "Quit: game aborted, no final score.\n";
      playing = false;
    } else if (command.size() == 1 && command[0] >= '1' && command[0] <= '5') {
      game = change_die_state(game, command[0] - '1');
      if (game.illegal_move.type != IllegalMoveType::None) print_error(out, game);
    } else {
      out << "Invalid command \"" << command << "\": use R, 1-5, S, or Q.\n";
    }
  }
  if (is_game_over(game)) {
    print_final_breakdown(out, game);
  }
  return game;
}

}  // namespace dice_party
