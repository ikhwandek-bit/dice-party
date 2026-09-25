// Phase 5 special-rules tests: Five-of-a-Kind Bonus + Joker (SPEC §11–§12).
// Standard library only, no terminal I/O.
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

void fill_slot(dice_party::GameState& g, dice_party::Category cat, int score) {
  const int index = static_cast<int>(cat);
  g.scorecard.slots[index].filled = true;
  g.scorecard.slots[index].score = score;
}

}  // namespace

int main() {
  using namespace dice_party;

  // A. Bonus, matching Upper free (§12.4 A): Yahtzee 50, roll 4x5 → Fours 20 + 100.
  {
    GameState g = rolled_state({4, 4, 4, 4, 4}, 1);
    fill_slot(g, Category::FiveOfAKind, 50);
    EXPECT(is_five_of_a_kind_bonus(g), "A is bonus");
    EXPECT(must_select_category(g), "A must select immediately");
    EXPECT(can_select_category(g), "A can select");

    std::mt19937 gen(7);
    GameState reroll = roll_dice(g, gen);
    EXPECT(reroll.illegal_move.type == IllegalMoveType::BonusForcedStop,
           "A further roll forfeited");
    bool same = true;
    for (int i = 0; i < kDieCount; ++i) same = same && (reroll.dice[i].face == 4);
    EXPECT(same, "A forfeited roll changes no faces");
    EXPECT_EQ(reroll.rolls_used, 1, "A forfeited roll consumes nothing");

    GameState wrong = select_category(g, Category::Chance);
    EXPECT(wrong.illegal_move.type == IllegalMoveType::JokerForcedCategory,
           "A wrong Joker choice rejected");
    EXPECT_EQ(wrong.five_of_a_kind_bonus_total, 0, "A rejected choice awards nothing");
    EXPECT(!wrong.scorecard.slots[static_cast<int>(Category::Chance)].filled,
           "A rejected choice fills nothing");

    g = select_category(g, Category::Fours);
    EXPECT(g.illegal_move.type == IllegalMoveType::None, "A forced Upper accepted");
    EXPECT_EQ(g.scorecard.slots[static_cast<int>(Category::Fours)].score, 20,
              "A Fours scores 5x4");
    EXPECT_EQ(g.five_of_a_kind_bonus_total, 100, "A awards +100");
    EXPECT_EQ(g.turn, 2, "A advances turn");
  }

  // B. Bonus, matching Upper used, Lower open (§12.4 B): roll 6x5, Lower Joker table.
  {
    struct Case {
      Category cat;
      int want;
      const char* name;
    };
    const Case cases[] = {
        {Category::FullHouse, 25, "B Full House Joker 25"},
        {Category::SmallStraight, 30, "B Small Straight Joker 30"},
        {Category::LargeStraight, 40, "B Large Straight Joker 40"},
        {Category::ThreeOfAKind, 30, "B Three of a Kind 5x6"},
        {Category::FourOfAKind, 30, "B Four of a Kind 5x6"},
        {Category::Chance, 30, "B Chance 5x6"},
    };
    for (const Case& c : cases) {
      GameState g = rolled_state({6, 6, 6, 6, 6}, 2);
      fill_slot(g, Category::FiveOfAKind, 50);
      fill_slot(g, Category::Sixes, 18);
      EXPECT(is_five_of_a_kind_bonus(g), "B is bonus");
      EXPECT(must_select_category(g), "B must select even with rolls left");
      g = select_category(g, c.cat);
      EXPECT(g.illegal_move.type == IllegalMoveType::None, c.name);
      EXPECT_EQ(g.scorecard.slots[static_cast<int>(c.cat)].score, c.want, c.name);
      EXPECT_EQ(g.five_of_a_kind_bonus_total, 100, "B awards +100");
    }
    // While matching Upper is used and Lower is open, another Upper is illegal.
    {
      GameState g = rolled_state({6, 6, 6, 6, 6}, 1);
      fill_slot(g, Category::FiveOfAKind, 50);
      fill_slot(g, Category::Sixes, 18);
      g = select_category(g, Category::Ones);
      EXPECT(g.illegal_move.type == IllegalMoveType::JokerForcedCategory,
             "B Upper dump rejected while Lower open");
      EXPECT_EQ(g.five_of_a_kind_bonus_total, 0, "B rejected dump awards nothing");
    }
  }

  // C. Bonus, only other Uppers left (§12.4 C): Lower full + Sixes used → dump 0.
  {
    GameState g = rolled_state({6, 6, 6, 6, 6}, 1);
    fill_slot(g, Category::FiveOfAKind, 50);
    fill_slot(g, Category::Sixes, 24);
    fill_slot(g, Category::ThreeOfAKind, 20);
    fill_slot(g, Category::FourOfAKind, 20);
    fill_slot(g, Category::FullHouse, 25);
    fill_slot(g, Category::SmallStraight, 30);
    fill_slot(g, Category::LargeStraight, 40);
    fill_slot(g, Category::Chance, 30);
    EXPECT(is_five_of_a_kind_bonus(g), "C is bonus");
    EXPECT(!lower_joker_has_open(g.scorecard), "C Lower full");
    g = select_category(g, Category::Ones);
    EXPECT(g.illegal_move.type == IllegalMoveType::None, "C Upper dump accepted");
    EXPECT_EQ(g.scorecard.slots[static_cast<int>(Category::Ones)].score, 0,
              "C dump scores 0");
    EXPECT_EQ(g.five_of_a_kind_bonus_total, 100, "C awards +100");
  }

  // D. No Joker when Yahtzee holds 0 (§12.4 D): normal §8 scoring, rerolls allowed.
  {
    GameState g = rolled_state({3, 3, 3, 3, 3}, 1);
    fill_slot(g, Category::FiveOfAKind, 0);
    EXPECT(!is_five_of_a_kind_bonus(g), "D no bonus when Yahtzee is 0");
    EXPECT(!must_select_category(g), "D no forced stop with rolls left");

    std::mt19937 gen(11);
    GameState reroll = roll_dice(g, gen);
    EXPECT(reroll.illegal_move.type == IllegalMoveType::None, "D reroll still allowed");

    GameState fh = g;
    fh = select_category(fh, Category::FullHouse);
    EXPECT(fh.illegal_move.type == IllegalMoveType::None, "D Full House dump legal");
    EXPECT_EQ(fh.scorecard.slots[static_cast<int>(Category::FullHouse)].score, 0,
              "D Full House scores 0 without Joker");
    EXPECT_EQ(fh.five_of_a_kind_bonus_total, 0, "D no +100");

    GameState up = g;
    up = select_category(up, Category::Threes);
    EXPECT_EQ(up.scorecard.slots[static_cast<int>(Category::Threes)].score, 15,
              "D Threes still scores 15");
    EXPECT_EQ(up.five_of_a_kind_bonus_total, 0, "D Upper awards no +100");
  }

  // E. First five-of-a-kind (§12.4 E): Yahtzee unused → no bonus, free choice.
  {
    GameState g = rolled_state({2, 2, 2, 2, 2}, 1);
    EXPECT(!is_five_of_a_kind_bonus(g), "E no bonus while Yahtzee unused");
    EXPECT(!must_select_category(g), "E no forced stop on first five");

    GameState y = g;
    y = select_category(y, Category::FiveOfAKind);
    EXPECT_EQ(y.scorecard.slots[static_cast<int>(Category::FiveOfAKind)].score, 50,
              "E Yahtzee scores 50");
    EXPECT_EQ(y.five_of_a_kind_bonus_total, 0, "E first Yahtzee awards no +100");

    GameState up = g;
    up = select_category(up, Category::Twos);
    EXPECT_EQ(up.scorecard.slots[static_cast<int>(Category::Twos)].score, 10,
              "E Twos scores 10");

    GameState tk = g;
    tk = select_category(tk, Category::ThreeOfAKind);
    EXPECT_EQ(tk.scorecard.slots[static_cast<int>(Category::ThreeOfAKind)].score, 10,
              "E Three of a Kind scores dice sum");

    GameState fh = g;
    fh = select_category(fh, Category::FullHouse);
    EXPECT_EQ(fh.scorecard.slots[static_cast<int>(Category::FullHouse)].score, 0,
              "E Full House rejects five of a kind without Joker");
  }

  // Multiple bonuses accrue on later turns; total includes them (§9).
  {
    GameState g = rolled_state({4, 4, 4, 4, 4}, 1);
    fill_slot(g, Category::FiveOfAKind, 50);
    g = select_category(g, Category::Fours);
    EXPECT_EQ(g.five_of_a_kind_bonus_total, 100, "multi first +100");
    for (int i = 0; i < kDieCount; ++i) g.dice[i].face = 6;
    g.rolls_used = 1;
    EXPECT(is_five_of_a_kind_bonus(g), "multi second is bonus");
    g = select_category(g, Category::Sixes);
    EXPECT_EQ(g.scorecard.slots[static_cast<int>(Category::Sixes)].score, 30,
              "multi Sixes 5x6");
    EXPECT_EQ(g.five_of_a_kind_bonus_total, 200, "multi second +100 accrues");
    EXPECT(total_score(g) >= 200, "multi total includes bonuses");
    EXPECT_EQ(total_score(g),
              upper_total(g.scorecard) + upper_bonus_points(g.scorecard) +
                  lower_total(g.scorecard) + 200,
              "multi total formula holds");
  }

  if (failures == 0) {
    std::cout << "PASS special-rules tests (" << checks << " checks)\n";
    return 0;
  }
  std::cout << failures << " of " << checks << " checks failed\n";
  return 1;
}
