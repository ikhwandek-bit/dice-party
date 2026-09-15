// Phase 2 dice tests. Standard library only, no terminal I/O.
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

bool faces_in_range(const dice_party::GameState& g) {
  for (int i = 0; i < dice_party::kDieCount; ++i) {
    if (g.dice[i].face < dice_party::kMinFace || g.dice[i].face > dice_party::kMaxFace) return false;
  }
  return true;
}

}  // namespace

int main() {
  using namespace dice_party;

  // 1. First roll rolls all five dice, faces 1-6.
  {
    std::mt19937 gen(42);
    GameState g = make_new_game();
    g = roll_dice(g, gen);
    EXPECT(g.rolls_used == 1, "first roll consumes one roll");
    EXPECT(faces_in_range(g), "first roll faces 1-6");
    EXPECT(g.illegal_move.type == IllegalMoveType::None, "first roll legal");
  }

  // 2. Locked dice keep faces; unlocked dice are re-rolled.
  {
    std::mt19937 gen(7);
    GameState g = make_new_game();
    g = roll_dice(g, gen);
    int locked_faces[kDieCount];
    for (int i = 0; i < kDieCount; ++i) locked_faces[i] = g.dice[i].face;
    g = change_die_state(g, 0);
    g = change_die_state(g, 2);
    EXPECT(g.dice[0].locked && g.dice[2].locked, "lock subset");
    g = roll_dice(g, gen);
    EXPECT(g.dice[0].face == locked_faces[0], "locked die 0 unchanged");
    EXPECT(g.dice[2].face == locked_faces[2], "locked die 2 unchanged");
    EXPECT(faces_in_range(g), "second roll faces 1-6");
    // Unlock and verify the die becomes rollable again.
    g = change_die_state(g, 0);
    EXPECT(!g.dice[0].locked, "unlock works");
  }

  // 3. Lock before first roll is rejected and observable.
  {
    GameState g = make_new_game();
    g = change_die_state(g, 1);
    EXPECT(g.illegal_move.type == IllegalMoveType::LockBeforeFirstRoll, "lock before roll rejected");
    EXPECT(!g.dice[1].locked, "pre-roll lock does not toggle");
    g = change_die_state(g, 99);
    EXPECT(g.illegal_move.type == IllegalMoveType::InvalidDieIndex, "bad index rejected");
    // Success clears the flag.
    std::mt19937 gen(9);
    g = roll_dice(g, gen);
    g = change_die_state(g, 1);
    EXPECT(g.dice[1].locked, "lock after roll works");
    EXPECT(g.illegal_move.type == IllegalMoveType::None, "success clears flag");
  }

  // 4. Fourth roll is rejected without changing state.
  {
    std::mt19937 gen(11);
    GameState g = make_new_game();
    g = roll_dice(g, gen);
    g = roll_dice(g, gen);
    g = roll_dice(g, gen);
    int faces[kDieCount];
    for (int i = 0; i < kDieCount; ++i) faces[i] = g.dice[i].face;
    g = roll_dice(g, gen);
    EXPECT(g.illegal_move.type == IllegalMoveType::MaxRollsReached, "fourth roll rejected");
    EXPECT(g.rolls_used == kMaxRollsPerTurn, "rolls_used stays at 3");
    bool same = true;
    for (int i = 0; i < kDieCount; ++i) same = same && (g.dice[i].face == faces[i]);
    EXPECT(same, "rejected roll changes no faces");
  }

  // 5. All-locked reroll is rejected without consuming a roll.
  {
    std::mt19937 gen(13);
    GameState g = make_new_game();
    g = roll_dice(g, gen);
    for (int i = 0; i < kDieCount; ++i) g = change_die_state(g, i);
    int used = g.rolls_used;
    int faces[kDieCount];
    for (int i = 0; i < kDieCount; ++i) faces[i] = g.dice[i].face;
    g = roll_dice(g, gen);
    EXPECT(g.illegal_move.type == IllegalMoveType::AllDiceLocked, "all-locked rejected");
    EXPECT(g.rolls_used == used, "rejected roll consumes nothing");
    bool same = true;
    for (int i = 0; i < kDieCount; ++i) same = same && (g.dice[i].face == faces[i]);
    EXPECT(same, "all-locked changes no faces");
  }

  // 6. Reject priority: all-locked wins when both it and max-rolls hold.
  {
    std::mt19937 gen(17);
    GameState g = make_new_game();
    g = roll_dice(g, gen);
    g = roll_dice(g, gen);
    g = roll_dice(g, gen);
    for (int i = 0; i < kDieCount; ++i) g = change_die_state(g, i);
    g = roll_dice(g, gen);
    EXPECT(g.illegal_move.type == IllegalMoveType::AllDiceLocked, "all-locked priority");
  }

  // 7. New turn resets locks/faces/rolls and clears stale flags.
  {
    std::mt19937 gen(19);
    GameState g = make_new_game();
    g = roll_dice(g, gen);
    g = change_die_state(g, 0);
    g = new_turn(g);
    EXPECT(g.turn == 2, "turn advances");
    EXPECT(g.rolls_used == 0, "rolls reset");
    EXPECT(g.illegal_move.type == IllegalMoveType::None, "new turn clears flag");
    bool clean = true;
    for (int i = 0; i < kDieCount; ++i) clean = clean && !g.dice[i].locked && g.dice[i].face == 0;
    EXPECT(clean, "locks and faces reset");
    g.turn = kTurnCount;
    g = new_turn(g);
    EXPECT(g.illegal_move.type == IllegalMoveType::MaxTurnsReached, "turn 14 blocked");
    EXPECT(g.turn == kTurnCount, "turn stays at 13");
  }

  if (failures == 0) {
    std::cout << "PASS dice tests (" << checks << " checks)\n";
    return 0;
  }
  std::cout << failures << " of " << checks << " checks failed\n";
  return 1;
}
