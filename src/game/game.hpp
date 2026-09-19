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

enum class IllegalMoveType {
  None = 0,
  AllDiceLocked = 1,
  MaxRollsReached = 2,
  MaxTurnsReached = 3,
  LockBeforeFirstRoll = 4,
  InvalidDieIndex = 5
};

struct IllegalMove {
  IllegalMoveType type = IllegalMoveType::None;
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
  game.illegal_move.type = IllegalMoveType::None; // Reset illegal move type at the start of the roll
  for (int i = 0; i < kDieCount; ++i) {
    if (game.dice[i].locked) {
      ++locked_count;
    }
  }

  // Reject priority: all-locked is reported before max-rolls when both hold.
  // Either way the roll is rejected without changing faces or consuming a roll.
  if (locked_count == kDieCount) {
    game.illegal_move.type = IllegalMoveType::AllDiceLocked;
    return game; // All dice are locked, no need to roll
  }

  if (game.rolls_used >= kMaxRollsPerTurn) {
    game.illegal_move.type = IllegalMoveType::MaxRollsReached;
    return game; // Maximum rolls reached, no need to roll
  }

  for (int i = 0; i < kDieCount; ++i) {
    if (!game.dice[i].locked) {
      game.dice[i].face = std::uniform_int_distribution<int>(kMinFace, kMaxFace)(gen); // Generates a random number between 1 and 6
    }
  }
  ++game.rolls_used;
  return game;
} 

inline GameState change_die_state(GameState game, int die_index) {
  if (die_index < 0 || die_index >= kDieCount) {
    game.illegal_move.type = IllegalMoveType::InvalidDieIndex;
    return game;
  }
  if (game.rolls_used == 0) {
    game.illegal_move.type = IllegalMoveType::LockBeforeFirstRoll;
    return game;
  }
  game.dice[die_index].locked = !game.dice[die_index].locked;
  game.illegal_move.type = IllegalMoveType::None;
  return game;
}

inline GameState new_turn(GameState game) {
  if (game.turn >= kTurnCount) {
    game.illegal_move.type = IllegalMoveType::MaxTurnsReached;
    return game; // Maximum turns reached, no need to start a new turn
  }

  for (int i = 0; i < kDieCount; ++i) {
    game.dice[i].face = 0; // Reset dice faces
    game.dice[i].locked = false; // Unlock all dice
  }
  game.rolls_used = 0;
  game.illegal_move.type = IllegalMoveType::None;
  ++game.turn;
  return game;
}

namespace detail {

inline bool has_three_of_a_kind(const GameState& g) {
  for (int f = kMinFace; f <= kMaxFace; ++f) {
    int count = 0;
    for (int i = 0; i < kDieCount; ++i) {
      if (g.dice[i].face == f) ++count;
    }
    if (count >= 3) return true;
  }
  return false;
}

inline bool has_four_of_a_kind(const GameState& g) {
  for (int f = kMinFace; f <= kMaxFace; ++f) {
    int count = 0;
    for (int i = 0; i < kDieCount; ++i) {
      if (g.dice[i].face == f) ++count;
    }
    if (count >= 4) return true;
  }
  return false;
}

inline bool has_full_house(const GameState& g) {
  int three_count = 0, two_count = 0;
  for (int f = kMinFace; f <= kMaxFace; ++f) {
    int count = 0;
    for (int i = 0; i < kDieCount; ++i) {
      if (g.dice[i].face == f) ++count;
    }
    if (count >= 3) ++three_count;
    else if (count == 2) ++two_count;
  }
  return three_count >= 1 && two_count >= 1;
}

inline bool has_small_straight(const GameState& g) {
  bool has1 = false, has2 = false, has3 = false, has4 = false, has5 = false, has6 = false;
  for (int i = 0; i < kDieCount; ++i) {
    int f = g.dice[i].face;
    if (f == 1) has1 = true;
    else if (f == 2) has2 = true;
    else if (f == 3) has3 = true;
    else if (f == 4) has4 = true;
    else if (f == 5) has5 = true;
    else if (f == 6) has6 = true;
  }
  return (has1 && has2 && has3 && has4) ||
         (has2 && has3 && has4 && has5) ||
         (has3 && has4 && has5 && has6);
}

inline bool has_large_straight(const GameState& g) {
  bool has1 = false, has2 = false, has3 = false, has4 = false, has5 = false, has6 = false;
  for (int i = 0; i < kDieCount; ++i) {
    int f = g.dice[i].face;
    if (f == 1) has1 = true;
    else if (f == 2) has2 = true;
    else if (f == 3) has3 = true;
    else if (f == 4) has4 = true;
    else if (f == 5) has5 = true;
    else if (f == 6) has6 = true;
  }
  return (has1 && has2 && has3 && has4 && has5) ||
         (has2 && has3 && has4 && has5 && has6);
}

} // namespace detail

inline int score_ones(const GameState& g) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (g.dice[i].face == 1) sum += 1;
  }
  return sum;
}

inline int score_twos(const GameState& g) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (g.dice[i].face == 2) sum += 2;
  }
  return sum;
}

inline int score_threes(const GameState& g) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (g.dice[i].face == 3) sum += 3;
  }
  return sum;
}

inline int score_fours(const GameState& g) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (g.dice[i].face == 4) sum += 4;
  }
  return sum;
}

inline int score_fives(const GameState& g) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (g.dice[i].face == 5) sum += 5;
  }
  return sum;
}

inline int score_sixes(const GameState& g) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (g.dice[i].face == 6) sum += 6;
  }
  return sum;
}

inline bool upper_bonus_earned(const Scorecard& card) {
  int total = 0;
  for (int i = 0; i <= static_cast<int>(Category::Sixes); ++i) {
    total += card.slots[i].score;
  }
  return total >= kUpperBonusThreshold;
}

inline int score_three_of_a_kind(const GameState& g) {
  if (detail::has_three_of_a_kind(g)) {
    int sum = 0;
    for (int i = 0; i < kDieCount; ++i) sum += g.dice[i].face;
    return sum;
  }
  return 0;
}

inline int score_four_of_a_kind(const GameState& g) {
  if (detail::has_four_of_a_kind(g)) {
    int sum = 0;
    for (int i = 0; i < kDieCount; ++i) sum += g.dice[i].face;
    return sum;
  }
  return 0;
}

inline int score_full_house(const GameState& g) {
  if (detail::has_full_house(g)) return 25;
  return 0;
}

inline int score_small_straight(const GameState& g) {
  if (detail::has_small_straight(g)) return 30;
  return 0;
}

inline int score_large_straight(const GameState& g) {
  if (detail::has_large_straight(g)) return 40;
  return 0;
}

inline int score_five_of_a_kind(const GameState& g) {
  for (int f = kMinFace; f <= kMaxFace; ++f) {
    int count = 0;
    for (int i = 0; i < kDieCount; ++i) {
      if (g.dice[i].face == f) ++count;
    }
    if (count == kDieCount) return 50; // Five of a Kind category scores 50
  }
  return 0;
}

inline int score_chance(const GameState& g) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) sum += g.dice[i].face;
  return sum;
}

}// namespace dice_party
