#pragma once
#include <random>

// Core game data. No terminal I/O belongs in this header.

namespace dice_party {

inline constexpr int kDieCount = 5;
inline constexpr int kMinFace = 1;
inline constexpr int kMaxFace = 6;
inline constexpr int kMaxRollsPerTurn = 3;
inline constexpr int kTurnCount = 13;
inline constexpr int kCategoryCount = 13;
inline constexpr int kUpperBonusThreshold = 63;
inline constexpr int kUpperBonusPoints = 35;
inline constexpr int kFiveOfAKindBonusPoints = 100;

enum class Category {
  Ones = 0,
  Twos,
  Threes,
  Fours,
  Fives,
  Sixes,
  ThreeOfAKind,
  FourOfAKind,
  FullHouse,
  SmallStraight,
  LargeStraight,
  FiveOfAKind,
  Chance,
};

struct Die {
  int face = 0;  // 0 = not rolled this turn; otherwise 1–6
  bool locked = false;
};

struct CategorySlot {
  bool filled = false;
  int score = 0;
};

struct Scorecard {
  CategorySlot slots[kCategoryCount]{};
};

struct IllegalMove {
  int type = 0; // 0 = none, 1 = all dice locked, 2 = max rolls reached, 3 = max turns reached
};

struct GameState {
  Die dice[kDieCount]{};
  Scorecard scorecard{};
  IllegalMove illegal_move{};
  int turn = 1;        // 1–13
  int rolls_used = 0;  // 0–3 in the current turn
  int five_of_a_kind_bonus_total = 0;
};

inline GameState make_new_game() {
  return GameState{};
}

inline int unused_category_count(const Scorecard& card) {
  int unused = 0;
  for (int i = 0; i < kCategoryCount; ++i) {
    if (!card.slots[i].filled) {
      ++unused;
    }
  }
  return unused;
}

inline GameState roll_dice(GameState game, std::mt19937& gen) {
  int locked_count = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (game.dice[i].locked) {
      ++locked_count;
    }
  }

  if (locked_count == kDieCount) {
    game.illegal_move.type = 1;
    return game; // All dice are locked, no need to roll
  }

  if (game.rolls_used >= kMaxRollsPerTurn) {
    game.illegal_move.type = 2;
    return game; // Maximum rolls reached, no need to roll
  }

  for (int i = 0; i < kDieCount; ++i) {
    if (!game.dice[i].locked) {
      game.dice[i].face = gen() % 6 + 1; // Generates a random number between 1 and 6
    }
  }
  ++game.rolls_used;
  return game;
} 

inline GameState change_die_state(GameState game, int die_index) {
  if (die_index >= 0 && die_index < kDieCount) {
    game.dice[die_index].locked = !game.dice[die_index].locked;
  }
  return game;
}

inline GameState new_turn(GameState game) {
  if (game.turn >= kTurnCount) {
    game.illegal_move.type = 3;
    return game; // Maximum turns reached, no need to start a new turn
  }

  for (int i = 0; i < kDieCount; ++i) {
    game.dice[i].face = 0; // Reset dice faces
    game.dice[i].locked = false; // Unlock all dice
  }
  game.rolls_used = 0;
  ++game.turn;
  return game;
}

}// namespace dice_party
