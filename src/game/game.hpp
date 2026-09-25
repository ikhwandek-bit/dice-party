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
  InvalidDieIndex = 5,
  NoRollYet = 6,
  CategoryAlreadyFilled = 7,
  InvalidCategory = 8,
  GameAlreadyOver = 9,
  BonusForcedStop = 10,
  JokerForcedCategory = 11
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

// --- Phase 5: Five-of-a-Kind Bonus detection (SPEC §11) ---
// Bonus applies iff dice are five of a kind AND Five of a Kind holds 50.
// Defined before roll_dice so the forced-stop check can use it.
inline int current_five_face(const GameState& g) {
  const int first = g.dice[0].face;
  if (first < kMinFace || first > kMaxFace) return 0;
  for (int i = 1; i < kDieCount; ++i) {
    if (g.dice[i].face != first) return 0;
  }
  return first;
}

inline bool is_five_of_a_kind_bonus(const GameState& g) {
  if (current_five_face(g) == 0) return false;
  const CategorySlot& yahtzee = g.scorecard.slots[static_cast<int>(Category::FiveOfAKind)];
  return yahtzee.filled && yahtzee.score == 50;
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
  // Phase 5 forced-stop (SPEC §11): a bonus five-of-a-kind forfeits remaining rolls.
  if (is_five_of_a_kind_bonus(game)) {
    game.illegal_move.type = IllegalMoveType::BonusForcedStop;
    return game;
  }
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

inline int count_face(const GameState& g, int face) {
  int count = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (g.dice[i].face == face) ++count;
  }
  return count;
}

inline bool has_three_of_a_kind(const GameState& g) {
  for (int f = kMinFace; f <= kMaxFace; ++f) {
    if (count_face(g, f) >= 3) return true;
  }
  return false;
}

inline bool has_four_of_a_kind(const GameState& g) {
  for (int f = kMinFace; f <= kMaxFace; ++f) {
    if (count_face(g, f) >= 4) return true;
  }
  return false;
}

inline bool has_five_of_a_kind(const GameState& g) {
  for (int f = kMinFace; f <= kMaxFace; ++f) {
    if (count_face(g, f) == kDieCount) return true;
  }
  return false;
}

inline bool has_full_house(const GameState& g) {
  int three_count = 0, two_count = 0;
  for (int f = kMinFace; f <= kMaxFace; ++f) {
    int count = count_face(g, f);
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

inline int score_upper(const GameState& g, int face) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) {
    if (g.dice[i].face == face) sum += face;
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
  if (detail::has_five_of_a_kind(g)) return 50;
  return 0;
}

inline int score_chance(const GameState& g) {
  int sum = 0;
  for (int i = 0; i < kDieCount; ++i) sum += g.dice[i].face;
  return sum;
}

// --- Phase 4: game flow (SPEC §4, §5, §9; normal §8 scoring, no Joker) ---

inline int score_for_category(const GameState& g, Category category) {
  switch (category) {
    case Category::Ones: return score_upper(g, 1);
    case Category::Twos: return score_upper(g, 2);
    case Category::Threes: return score_upper(g, 3);
    case Category::Fours: return score_upper(g, 4);
    case Category::Fives: return score_upper(g, 5);
    case Category::Sixes: return score_upper(g, 6);
    case Category::ThreeOfAKind: return score_three_of_a_kind(g);
    case Category::FourOfAKind: return score_four_of_a_kind(g);
    case Category::FullHouse: return score_full_house(g);
    case Category::SmallStraight: return score_small_straight(g);
    case Category::LargeStraight: return score_large_straight(g);
    case Category::FiveOfAKind: return score_five_of_a_kind(g);
    case Category::Chance: return score_chance(g);
  }
  return 0;
}

inline int filled_count(const Scorecard& card) {
  int filled = 0;
  for (int i = 0; i < kCategoryCount; ++i) {
    if (card.slots[i].filled) ++filled;
  }
  return filled;
}

inline bool is_game_over(const GameState& game) {
  return filled_count(game.scorecard) >= kCategoryCount;
}

// A category can be taken after the first roll of the turn (§5.4).
inline bool can_select_category(const GameState& game) {
  return !is_game_over(game) && game.rolls_used >= 1;
}

// After the third roll the player must choose a category (§5.5).
// A Five-of-a-Kind Bonus also forces an immediate choice (§11).
inline bool must_select_category(const GameState& game) {
  if (is_game_over(game)) return false;
  if (is_five_of_a_kind_bonus(game)) return true;
  return game.rolls_used >= kMaxRollsPerTurn;
}

// --- Phase 5: Joker placement (SPEC §12) ---

inline Category joker_matching_upper(int face) {
  return static_cast<Category>(face - 1);  // Ones + (F - 1), face 1–6
}

inline bool is_upper_category(Category category) {
  const int index = static_cast<int>(category);
  return index >= static_cast<int>(Category::Ones) && index <= static_cast<int>(Category::Sixes);
}

inline bool is_lower_joker_eligible(Category category) {
  switch (category) {
    case Category::ThreeOfAKind:
    case Category::FourOfAKind:
    case Category::FullHouse:
    case Category::SmallStraight:
    case Category::LargeStraight:
    case Category::Chance:
      return true;
    default:
      return false;
  }
}

inline bool lower_joker_has_open(const Scorecard& card) {
  for (int i = static_cast<int>(Category::ThreeOfAKind); i < kCategoryCount; ++i) {
    if (i == static_cast<int>(Category::FiveOfAKind)) continue;
    if (!card.slots[i].filled) return true;
  }
  return false;
}

// Score for a bonus turn under Joker table §12.2. Face must be 1–6.
inline int joker_score_for(Category category, int face) {
  if (is_upper_category(category)) {
    return (category == joker_matching_upper(face)) ? 5 * face : 0;
  }
  switch (category) {
    case Category::ThreeOfAKind:
    case Category::FourOfAKind:
    case Category::Chance:
      return 5 * face;
    case Category::FullHouse:
      return 25;
    case Category::SmallStraight:
      return 30;
    case Category::LargeStraight:
      return 40;
    default:
      return 0;
  }
}

// Whether a category choice is legal under the forced Joker order §12.1.
// Assumes basic guards (game over, valid index, unused, rolled) already hold.
inline bool is_joker_choice_legal(const GameState& game, Category category) {
  if (!is_five_of_a_kind_bonus(game)) return true;
  const int face = current_five_face(game);
  const Category matching = joker_matching_upper(face);
  const bool matching_unused =
      !game.scorecard.slots[static_cast<int>(matching)].filled;
  if (matching_unused) {
    return category == matching;
  }
  if (lower_joker_has_open(game.scorecard)) {
    return is_lower_joker_eligible(category);
  }
  return is_upper_category(category);
}

inline int upper_total(const Scorecard& card) {
  int total = 0;
  for (int i = 0; i <= static_cast<int>(Category::Sixes); ++i) {
    total += card.slots[i].score;
  }
  return total;
}

inline int lower_total(const Scorecard& card) {
  int total = 0;
  for (int i = static_cast<int>(Category::ThreeOfAKind); i < kCategoryCount; ++i) {
    total += card.slots[i].score;
  }
  return total;
}

inline int upper_bonus_points(const Scorecard& card) {
  return upper_bonus_earned(card) ? kUpperBonusPoints : 0;
}

// SPEC §9: Upper + Upper Bonus + Lower + Five-of-a-Kind bonuses.
inline int total_score(const GameState& game) {
  return upper_total(game.scorecard) + upper_bonus_points(game.scorecard) +
         lower_total(game.scorecard) + game.five_of_a_kind_bonus_total;
}

// Fill exactly one unused category with the current dice (§5.6, §8, §11–§12).
// On success advances to the next turn, except after the 13th fill.
inline GameState select_category(GameState game, Category category) {
  const int index = static_cast<int>(category);
  if (is_game_over(game)) {
    game.illegal_move.type = IllegalMoveType::GameAlreadyOver;
    return game;
  }
  if (index < 0 || index >= kCategoryCount) {
    game.illegal_move.type = IllegalMoveType::InvalidCategory;
    return game;
  }
  if (game.scorecard.slots[index].filled) {
    game.illegal_move.type = IllegalMoveType::CategoryAlreadyFilled;
    return game;
  }
  if (game.rolls_used < 1) {
    game.illegal_move.type = IllegalMoveType::NoRollYet;
    return game;
  }
  if (is_five_of_a_kind_bonus(game)) {
    if (!is_joker_choice_legal(game, category)) {
      game.illegal_move.type = IllegalMoveType::JokerForcedCategory;
      return game;
    }
    game.scorecard.slots[index].score = joker_score_for(category, current_five_face(game));
    game.scorecard.slots[index].filled = true;
    game.five_of_a_kind_bonus_total += kFiveOfAKindBonusPoints;
    game.illegal_move.type = IllegalMoveType::None;
  } else {
    game.scorecard.slots[index].score = score_for_category(game, category);
    game.scorecard.slots[index].filled = true;
    game.illegal_move.type = IllegalMoveType::None;
  }
  if (is_game_over(game)) {
    return game;
  }
  for (int i = 0; i < kDieCount; ++i) {
    game.dice[i].face = 0;
    game.dice[i].locked = false;
  }
  game.rolls_used = 0;
  ++game.turn;
  return game;
}

}// namespace dice_party
