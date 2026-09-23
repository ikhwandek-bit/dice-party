// Phase 3 scoring tests. Standard library only, no terminal I/O.
// Covers each category's valid, invalid, and 0-score cases, including SPEC.md examples.
#include <iostream>
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

dice_party::GameState make_state(std::initializer_list<int> faces) {
  dice_party::GameState g = dice_party::make_new_game();
  int i = 0;
  for (int f : faces) {
    g.dice[i].face = f;
    ++i;
  }
  return g;
}

dice_party::Scorecard make_upper(int ones, int twos, int threes, int fours, int fives,
                                 int sixes) {
  dice_party::Scorecard card{};
  card.slots[static_cast<int>(dice_party::Category::Ones)].score = ones;
  card.slots[static_cast<int>(dice_party::Category::Twos)].score = twos;
  card.slots[static_cast<int>(dice_party::Category::Threes)].score = threes;
  card.slots[static_cast<int>(dice_party::Category::Fours)].score = fours;
  card.slots[static_cast<int>(dice_party::Category::Fives)].score = fives;
  card.slots[static_cast<int>(dice_party::Category::Sixes)].score = sixes;
  return card;
}

}  // namespace

int main() {
  using namespace dice_party;

  // --- Upper Section: valid (SPEC §7.1 example) ---
  EXPECT_EQ(score_upper(make_state({6, 6, 1, 3, 6}), 6), 18, "sixes SPEC example 6,6,1,3,6 -> 18");
  EXPECT_EQ(score_upper(make_state({1, 1, 3, 4, 6}), 1), 2, "ones partial");
  EXPECT_EQ(score_upper(make_state({2, 2, 2, 4, 5}), 2), 6, "twos valid");
  EXPECT_EQ(score_upper(make_state({3, 3, 3, 3, 5}), 3), 12, "threes valid");
  EXPECT_EQ(score_upper(make_state({4, 4, 1, 2, 6}), 4), 8, "fours valid");
  EXPECT_EQ(score_upper(make_state({5, 5, 5, 1, 2}), 5), 15, "fives valid");
  EXPECT_EQ(score_upper(make_state({6, 1, 2, 3, 4}), 6), 6, "sixes single");

  // --- Upper Section: 0-score (no matching face) ---
  EXPECT_EQ(score_upper(make_state({2, 3, 4, 5, 6}), 1), 0, "ones zero");
  EXPECT_EQ(score_upper(make_state({1, 3, 4, 5, 6}), 2), 0, "twos zero");
  EXPECT_EQ(score_upper(make_state({1, 2, 4, 5, 6}), 3), 0, "threes zero");
  EXPECT_EQ(score_upper(make_state({1, 2, 3, 5, 6}), 4), 0, "fours zero");
  EXPECT_EQ(score_upper(make_state({1, 2, 3, 4, 6}), 5), 0, "fives zero");
  EXPECT_EQ(score_upper(make_state({1, 2, 3, 4, 5}), 6), 0, "sixes zero");
  // --- Upper Section: invalid-as-dump (five of a kind of another face scores 0) ---
  EXPECT_EQ(score_upper(make_state({6, 6, 6, 6, 6}), 1), 0, "ones invalid vs five sixes");
  EXPECT_EQ(score_upper(make_state({1, 1, 1, 1, 1}), 6), 0, "sixes invalid vs five ones");

  // --- Upper Bonus (§7.2): 35 iff Upper sum >= 63 ---
  EXPECT(upper_bonus_earned(make_upper(3, 6, 9, 12, 15, 18)), "bonus at exactly 63");  // 3*(1+..+6)=63
  EXPECT(upper_bonus_earned(make_upper(5, 10, 15, 20, 25, 30)), "bonus above 63");
  EXPECT(!upper_bonus_earned(make_upper(3, 6, 9, 12, 15, 17)), "no bonus at 62");
  EXPECT(!upper_bonus_earned(make_upper(0, 0, 0, 0, 0, 0)), "no bonus at 0");

  // --- Three of a Kind (§7.3 SPEC example 1,4,4,4,5 -> 18) ---
  EXPECT_EQ(score_three_of_a_kind(make_state({1, 4, 4, 4, 5})), 18, "3oak SPEC example");
  EXPECT_EQ(score_three_of_a_kind(make_state({2, 2, 4, 2, 2})), 12, "3oak four of a kind also valid");
  EXPECT_EQ(score_three_of_a_kind(make_state({6, 6, 6, 6, 6})), 30, "3oak five of a kind valid (§8)");
  EXPECT_EQ(score_three_of_a_kind(make_state({1, 2, 3, 4, 5})), 0, "3oak invalid straight");
  EXPECT_EQ(score_three_of_a_kind(make_state({1, 2, 3, 5, 6})), 0, "3oak invalid no triple");
  EXPECT_EQ(score_three_of_a_kind(make_state({2, 2, 3, 3, 4})), 0, "3oak invalid two pair");

  // --- Four of a Kind (§7.3 SPEC example 2,2,4,2,2 -> 12) ---
  EXPECT_EQ(score_four_of_a_kind(make_state({2, 2, 4, 2, 2})), 12, "4oak SPEC example");
  EXPECT_EQ(score_four_of_a_kind(make_state({6, 6, 6, 6, 6})), 30, "4oak five of a kind valid (§8)");
  EXPECT_EQ(score_four_of_a_kind(make_state({1, 4, 4, 4, 5})), 0, "4oak invalid only triple");
  EXPECT_EQ(score_four_of_a_kind(make_state({1, 2, 3, 4, 5})), 0, "4oak invalid straight");
  EXPECT_EQ(score_four_of_a_kind(make_state({2, 2, 3, 3, 3})), 0, "4oak invalid full house");

  // --- Full House (§7.3: 25 valid; five of a kind rejected) ---
  EXPECT_EQ(score_full_house(make_state({2, 2, 3, 3, 3})), 25, "full house valid 2,2,3,3,3");
  EXPECT_EQ(score_full_house(make_state({1, 1, 1, 5, 5})), 25, "full house valid 1,1,1,5,5");
  EXPECT_EQ(score_full_house(make_state({5, 5, 5, 5, 5})), 0, "full house invalid five of a kind (§8)");
  EXPECT_EQ(score_full_house(make_state({1, 4, 4, 4, 5})), 0, "full house invalid triple only");
  EXPECT_EQ(score_full_house(make_state({1, 2, 3, 4, 5})), 0, "full house invalid straight");
  EXPECT_EQ(score_full_house(make_state({2, 2, 2, 2, 5})), 0, "full house invalid four of a kind");

  // --- Small Straight (§7.3: 30; SPEC examples) ---
  EXPECT_EQ(score_small_straight(make_state({3, 2, 4, 5, 3})), 30, "small straight SPEC 3,2,4,5,3");
  EXPECT_EQ(score_small_straight(make_state({1, 2, 3, 4, 4})), 30, "small straight SPEC 1,2,3,4,4");
  EXPECT_EQ(score_small_straight(make_state({1, 2, 3, 4, 6})), 30, "small straight SPEC 1,2,3,4,6");
  EXPECT_EQ(score_small_straight(make_state({2, 3, 4, 5, 6})), 30, "small straight 2-6 run");
  EXPECT_EQ(score_small_straight(make_state({1, 2, 3, 5, 6})), 0, "small straight SPEC invalid 1,2,3,5,6");
  EXPECT_EQ(score_small_straight(make_state({1, 1, 3, 5, 6})), 0, "small straight invalid no run");
  EXPECT_EQ(score_small_straight(make_state({4, 4, 4, 4, 4})), 0, "small straight invalid five of a kind (§8)");

  // --- Large Straight (§7.3: 40 only for 1-5 and 2-6) ---
  EXPECT_EQ(score_large_straight(make_state({1, 2, 3, 4, 5})), 40, "large straight 1-5");
  EXPECT_EQ(score_large_straight(make_state({2, 3, 4, 5, 6})), 40, "large straight 2-6");
  EXPECT_EQ(score_large_straight(make_state({1, 2, 3, 4, 6})), 0, "large straight invalid 1,2,3,4,6");
  EXPECT_EQ(score_large_straight(make_state({3, 2, 4, 5, 3})), 0, "large straight invalid small only");
  EXPECT_EQ(score_large_straight(make_state({1, 2, 3, 5, 6})), 0, "large straight invalid gap");
  EXPECT_EQ(score_large_straight(make_state({5, 5, 5, 5, 5})), 0, "large straight invalid five of a kind (§8)");

  // --- Five of a Kind (§7.3: 50 or 0) ---
  EXPECT_EQ(score_five_of_a_kind(make_state({4, 4, 4, 4, 4})), 50, "five of a kind valid");
  EXPECT_EQ(score_five_of_a_kind(make_state({1, 1, 1, 1, 1})), 50, "five of a kind valid ones");
  EXPECT_EQ(score_five_of_a_kind(make_state({2, 2, 4, 2, 2})), 0, "five of a kind invalid four");
  EXPECT_EQ(score_five_of_a_kind(make_state({2, 2, 3, 3, 3})), 0, "five of a kind invalid house");
  EXPECT_EQ(score_five_of_a_kind(make_state({1, 2, 3, 4, 5})), 0, "five of a kind invalid straight");

  // --- Chance (§7.3: sum of all five, always scores) ---
  EXPECT_EQ(score_chance(make_state({1, 2, 3, 4, 5})), 15, "chance 1-5");
  EXPECT_EQ(score_chance(make_state({6, 6, 6, 6, 6})), 30, "chance five sixes");
  EXPECT_EQ(score_chance(make_state({1, 1, 1, 1, 1})), 5, "chance five ones (minimum)");
  EXPECT_EQ(score_chance(make_state({1, 4, 4, 4, 5})), 18, "chance mixed");

  if (failures == 0) {
    std::cout << "PASS scoring tests (" << checks << " checks)\n";
    return 0;
  }
  std::cout << failures << " of " << checks << " checks failed\n";
  return 1;
}
