// Phase 4 game-flow tests. Standard library only, no terminal I/O.
#include <iostream>
#include <random>
#include <string>

#include "game/game.hpp"

namespace {

int failures = 0;
int checks = 0;

void expect(bool cond, const std::string& name, int line) {
  ++checks;
  if (!cond) {
    ++failures;
    std::cout << "FAIL " << name << " (line " << line << ")\n";
  }
}

#define EXPECT(cond, name) expect((cond), (name), __LINE__)
#define EXPECT_EQ(actual, expected, name) \
  expect((actual) == (expected), std::string(name) + " got " + std::to_string(actual) + \
           " want " + std::to_string(expected), __LINE__)

dice_party::GameState rolled_state(std::initializer_list<int> faces, int rolls_used = 1) {
  dice_party::GameState g = dice_party::make_new_game();
  int i = 0;
  for (int f : faces) {
    g.dice[i].face = f;
    ++i;
  }
  g.rolls_used = rolls_used;
  return g;
}

}  // namespace

int main() {
  using namespace dice_party;

  // 1. New game starts with 13 unused categories, turn 1, no rolls, total 0.
  {
    GameState g = make_new_game();
    EXPECT_EQ(unused_category_count(g.scorecard), kCategoryCount, "13 unused at start");
    EXPECT_EQ(filled_count(g.scorecard), 0, "0 filled at start");
    EXPECT_EQ(g.turn, 1, "turn 1 at start");
    EXPECT_EQ(g.rolls_used, 0, "0 rolls at start");
    EXPECT(!is_game_over(g), "not over at start");
    EXPECT_EQ(total_score(g), 0, "total 0 at start");
  }

  // 2. Category selection requires a first roll (§5.2).
  {
    GameState g = make_new_game();
    EXPECT(!can_select_category(g), "cannot select before first roll");
    g = select_category(g, Category::Chance);
    EXPECT(g.illegal_move.type == IllegalMoveType::NoRollYet, "select before roll rejected");
    EXPECT(!g.scorecard.slots[static_cast<int>(Category::Chance)].filled,
           "rejected select fills nothing");
    EXPECT_EQ(g.turn, 1, "rejected select keeps turn");
  }

  // 3. Selection allowed after roll 1, 2, or 3 (§5.4).
  for (int rolls : {1, 2, 3}) {
    GameState g = rolled_state({1, 2, 3, 4, 5}, rolls);
    EXPECT(can_select_category(g), "can select after roll");
    g = select_category(g, Category::Chance);
    EXPECT(g.illegal_move.type == IllegalMoveType::None, "select after roll legal");
    EXPECT(g.scorecard.slots[static_cast<int>(Category::Chance)].filled, "chance filled");
    EXPECT_EQ(g.scorecard.slots[static_cast<int>(Category::Chance)].score, 15,
              "chance scores dice sum");
  }

  // 4. After roll 3 the player must choose (§5.5): no 4th roll, must_select set.
  {
    std::mt19937 gen(21);
    GameState g = make_new_game();
    g = roll_dice(g, gen);
    g = roll_dice(g, gen);
    g = roll_dice(g, gen);
    EXPECT(must_select_category(g), "must select after roll 3");
    const int faces[kDieCount] = {g.dice[0].face, g.dice[1].face, g.dice[2].face,
                                  g.dice[3].face, g.dice[4].face};
    g = roll_dice(g, gen);
    EXPECT(g.illegal_move.type == IllegalMoveType::MaxRollsReached, "fourth roll refused");
    bool same = true;
    for (int i = 0; i < kDieCount; ++i) same = same && (g.dice[i].face == faces[i]);
    EXPECT(same, "refused roll changes no faces");
    EXPECT(must_select_category(g), "still must select after refused roll");
    g = select_category(g, Category::Ones);
    EXPECT(g.illegal_move.type == IllegalMoveType::None, "post-roll-3 select works");
  }
  {
    GameState early = rolled_state({1, 1, 2, 3, 4}, 1);
    EXPECT(!must_select_category(early), "no forced select after roll 1");
  }

  // 5. A used category cannot be selected again (§5.6).
  {
    GameState g = rolled_state({1, 1, 2, 3, 4}, 1);
    g = select_category(g, Category::Ones);
    EXPECT_EQ(g.scorecard.slots[static_cast<int>(Category::Ones)].score, 2, "ones scored");
    for (int i = 0; i < kDieCount; ++i) g.dice[i].face = (i < 2) ? 1 : 6;
    g.rolls_used = 1;
    const int turn_before = g.turn;
    g = select_category(g, Category::Ones);
    EXPECT(g.illegal_move.type == IllegalMoveType::CategoryAlreadyFilled, "duplicate rejected");
    EXPECT_EQ(g.scorecard.slots[static_cast<int>(Category::Ones)].score, 2,
              "duplicate keeps old score");
    EXPECT_EQ(g.turn, turn_before, "duplicate keeps turn");
  }

  // 6. A mismatched category may be filled with 0 (§5.7).
  {
    GameState g = rolled_state({1, 2, 3, 4, 5}, 2);
    g = select_category(g, Category::FullHouse);
    EXPECT(g.illegal_move.type == IllegalMoveType::None, "zero dump is legal");
    EXPECT(g.scorecard.slots[static_cast<int>(Category::FullHouse)].filled, "dump marks filled");
    EXPECT_EQ(g.scorecard.slots[static_cast<int>(Category::FullHouse)].score, 0, "dump scores 0");
  }

  // 7. Exactly 13 turns, one category per turn; game ends after the 13th fill (§4).
  //    Scripted full game with a known total (avoids a §11 bonus so the flow
  //    total stays Joker-free): Upper 3+6+9+12+15+18=63
  //    (+35 bonus) + Lower 18+12+25+30+40+50+29=204 → 63+35+204=302.
  //    (Chance uses {6,6,6,6,5}: five 6s there would trigger a +100 bonus
  //    now that Phase 5 is active.)
  {
    GameState g = make_new_game();
    const int faces[13][5] = {
        {1, 1, 1, 4, 5},  // Ones -> 3
        {2, 2, 2, 4, 5},  // Twos -> 6
        {3, 3, 3, 4, 5},  // Threes -> 9
        {4, 4, 4, 1, 2},  // Fours -> 12
        {5, 5, 5, 1, 2},  // Fives -> 15
        {6, 6, 6, 1, 2},  // Sixes -> 18
        {1, 4, 4, 4, 5},  // Three of a Kind -> 18 (SPEC example)
        {2, 2, 4, 2, 2},  // Four of a Kind -> 12 (SPEC example)
        {2, 2, 3, 3, 3},  // Full House -> 25
        {3, 2, 4, 5, 3},  // Small Straight -> 30 (SPEC example)
        {1, 2, 3, 4, 5},  // Large Straight -> 40
        {4, 4, 4, 4, 4},  // Five of a Kind -> 50
        {6, 6, 6, 6, 5},  // Chance -> 29 (not five of a kind: no §11 bonus)
    };
    const Category order[13] = {
        Category::Ones,         Category::Twos,         Category::Threes,
        Category::Fours,        Category::Fives,        Category::Sixes,
        Category::ThreeOfAKind, Category::FourOfAKind,  Category::FullHouse,
        Category::SmallStraight, Category::LargeStraight, Category::FiveOfAKind,
        Category::Chance,
    };
    for (int t = 0; t < 13; ++t) {
      EXPECT_EQ(g.turn, t + 1, "turn advances one per fill");
      for (int i = 0; i < kDieCount; ++i) g.dice[i].face = faces[t][i];
      g.rolls_used = 1;
      g = select_category(g, order[t]);
      EXPECT(g.illegal_move.type == IllegalMoveType::None, "scripted select legal");
      EXPECT_EQ(filled_count(g.scorecard), t + 1, "one category per turn");
    }
    EXPECT(is_game_over(g), "over after 13 fills");
    EXPECT_EQ(g.turn, kTurnCount, "ends on turn 13");
    EXPECT_EQ(upper_total(g.scorecard), 63, "upper subtotal 63");
    EXPECT_EQ(upper_bonus_points(g.scorecard), 35, "bonus at threshold");
    EXPECT_EQ(lower_total(g.scorecard), 204, "lower subtotal 204");
    EXPECT_EQ(total_score(g), 302, "final total 63+35+204");
    GameState extra = g;
    for (int i = 0; i < kDieCount; ++i) extra.dice[i].face = 6;
    extra.rolls_used = 1;
    extra = select_category(extra, Category::Ones);
    EXPECT(extra.illegal_move.type == IllegalMoveType::GameAlreadyOver, "14th fill rejected");
    EXPECT_EQ(total_score(extra), 302, "total unchanged after rejected fill");
    extra = new_turn(extra);
    EXPECT(extra.illegal_move.type == IllegalMoveType::MaxTurnsReached, "no turn past 13");
  }

  // 8. Final total without the bonus when Upper < 63 (§9).
  {
    GameState g = make_new_game();
    const int faces[13][5] = {
        {1, 2, 3, 4, 5},  // Ones -> 1
        {2, 3, 4, 5, 6},  // Twos -> 2
        {3, 4, 5, 6, 6},  // Threes -> 3
        {4, 5, 6, 6, 6},  // Fours -> 4
        {5, 6, 6, 6, 6},  // Fives -> 5
        {6, 1, 2, 3, 4},  // Sixes -> 6 (Upper = 21, no bonus)
        {1, 4, 4, 4, 5},  // Three of a Kind -> 18
        {2, 2, 4, 2, 2},  // Four of a Kind -> 12
        {2, 2, 3, 3, 3},  // Full House -> 25
        {1, 2, 3, 4, 6},  // Small Straight -> 30
        {2, 3, 4, 5, 6},  // Large Straight -> 40
        {1, 2, 3, 5, 6},  // Five of a Kind -> 0 dump
        {1, 1, 1, 1, 2},  // Chance -> 6
    };
    const Category order[13] = {
        Category::Ones,         Category::Twos,         Category::Threes,
        Category::Fours,        Category::Fives,        Category::Sixes,
        Category::ThreeOfAKind, Category::FourOfAKind,  Category::FullHouse,
        Category::SmallStraight, Category::LargeStraight, Category::FiveOfAKind,
        Category::Chance,
    };
    for (int t = 0; t < 13; ++t) {
      for (int i = 0; i < kDieCount; ++i) g.dice[i].face = faces[t][i];
      g.rolls_used = 1;
      g = select_category(g, order[t]);
    }
    EXPECT(is_game_over(g), "second script over");
    EXPECT_EQ(upper_total(g.scorecard), 21, "low upper subtotal");
    EXPECT_EQ(upper_bonus_points(g.scorecard), 0, "no bonus below 63");
    EXPECT_EQ(total_score(g), 21 + 18 + 12 + 25 + 30 + 40 + 0 + 6, "total without bonus");
  }

  if (failures == 0) {
    std::cout << "PASS game-flow tests (" << checks << " checks)\n";
    return 0;
  }
  std::cout << failures << " of " << checks << " checks failed\n";
  return 1;
}
