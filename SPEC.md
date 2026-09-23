# Dice-Party — Specification

> **Status:** Phase 4 In development  
> **Last updated:** 2026-09-23  
> **Version:** 0.4  

---

## 1. Overview

Dice-Party is a single-player, terminal-based dice game inspired by Yahtzee. The player rolls five six-sided dice over 13 turns and assigns each turn to exactly one unused scoring category. The goal is to maximize the final total after all 13 categories are filled.

This specification describes **what the game must do**. It does not prescribe C++ types, class layout, or UI widgets.

---

## 2. Game objective

Complete a 13-turn game by filling every scoring category once. The final score is the sum of:

- all Upper Section category scores
- the Upper Section Bonus, if earned
- all Lower Section category scores
- any Five-of-a-Kind Bonuses earned during the game

There is no second player in this version. “Highest total wins” means the player’s objective is to maximize that final total. The program must display the final total when the game ends.

---

## 3. Technical constraints

The product must:

- be written entirely in C++
- run only in a terminal / command-line environment
- not require a graphical user interface
- avoid unnecessary external libraries unless a later specification change justifies them
- keep game rules and scoring independent from terminal input/output where practical, so scoring and rules can be tested without driving a full interactive session

Out of scope for this version:

- networking
- graphical interfaces
- databases
- multiplayer / competing human or AI opponents
- persistence (save/load, leaderboards) unless added later

---

## 4. Game flow

1. The game starts with an empty scorecard (all 13 categories unused) and a total of 0.
2. The player plays exactly 13 turns. Each turn fills exactly one unused category.
3. A turn consists of rolling (and optionally rerolling) five dice, then assigning the final dice to one unused category.
4. After the 13th category is filled, the game computes bonuses that depend on the finished card (Upper Section Bonus is based on the six Upper totals; Five-of-a-Kind Bonuses are awarded during play when they occur).
5. The game ends and reports the final total. The player cannot start extra turns.

### 4.1 Command language

1. The player will use keys in order to play the game  
2. Key '**R**' is to roll or re-roll the dice  
3. Key '**1**' '**2**' '**3**' '**4**' '**5**' is to lock/unlocked the respective die
4. Key '**S**' is for the "scoring" tab
5. Inside the "scoring" tab the player can choose the category that they want to score  
6. The scoring key will follow the enum class (e.g '**0**' for ones, '**8**' for full house etc...)
7. The player can go back to the "dice" tab by pressing '**S**' again

---

## 5. Turn structure

Each turn:

1. All five dice start unlocked.
2. The player makes the first roll of all five dice. Every turn has at least this roll.
3. The player may make up to **three rolls total** in the turn (the initial roll plus up to two rerolls).
4. After the first roll, and after each later roll, the player may:
   - lock or unlock any dice, then reroll if rolls remain, or
   - choose an unused scoring category and end the turn
5. After the third roll, the player must choose an unused category. No further rerolls are allowed.
6. Exactly one category is used per turn. A used category cannot be selected again.
7. The player may select a category that the current dice do not satisfy. That category scores **0**.

**Five-of-a-Kind Bonus exception:** if a Five-of-a-Kind Bonus applies (see §11), the player must choose a category immediately and cannot use remaining rolls.

---

## 6. Dice rolling, locking, and limits

- There are always five dice. Each die shows an integer from 1 through 6.
- Each roll of an unlocked die is independent and uniformly random among 1–6.
- Locked dice keep their current face and are not rerolled.
- The player may lock any number of dice (including none or all) after a roll.
- The player may unlock previously locked dice before the next roll.
- Dice lock state does not survive into the next turn; a new turn starts with all dice unlocked.
- Maximum rolls per turn: **3**, except when a Five-of-a-Kind Bonus forces an immediate category choice.

Locking all five dice and then “rerolling” has no effect on faces (no unlocked dice change). The product may still consume a roll or reject the reroll; see Open Questions.

---

## 7. Scoring categories

There are 13 categories in two sections. Each may be used once.

### 7.1 Upper Section

Points equal the sum of dice showing the chosen face:

| Category | Score |
|---|---|
| Ones | sum of dice showing 1 |
| Twos | sum of dice showing 2 |
| Threes | sum of dice showing 3 |
| Fours | sum of dice showing 4 |
| Fives | sum of dice showing 5 |
| Sixes | sum of dice showing 6 |

Example: dice `6, 6, 1, 3, 6` scored as Sixes → `6 + 6 + 6 = 18`.

If the dice show none of that face, the category scores 0.

### 7.2 Upper Section Bonus

If the sum of the six Upper Section category scores is **63 or higher**, add **35** points.

63 is the conventional threshold (three of each face: `3×(1+2+3+4+5+6)`). Use 63 unless this specification is changed later.

The Upper Section Bonus:

- is not itself a category the player selects
- does not count toward the 13 turns
- is based only on the six Upper totals (not Lower scores, not Five-of-a-Kind Bonuses)

### 7.3 Lower Section

Dice may be considered in any order. “At least” means five-of-a-kind also satisfies three-of-a-kind and four-of-a-kind.

#### Three of a Kind

- Requirement: at least three dice share one face.
- Score: sum of all five dice, or 0 if the requirement fails.

Example: `1, 4, 4, 4, 5` → 18.

#### Four of a Kind

- Requirement: at least four dice share one face.
- Score: sum of all five dice, or 0 if the requirement fails.

Example: `2, 2, 4, 2, 2` → 12.

#### Full House

- Requirement: three dice of one face **and** two dice of a **different** face.
- Five of a kind does **not** count as a Full House under normal validation (Joker exception in §12).
- Score: **25** if valid, otherwise 0.

#### Small Straight

- Requirement: the dice contain **at least four consecutive** faces (duplicates allowed). Consecutive means a run of faces `n, n+1, n+2, n+3` among the five dice. Faces do not wrap (`6` then `1` is not consecutive).
- Example: `3, 2, 4, 5, 3` contains `2-3-4-5` → **30**.
- Other valid examples: `1,2,3,4,4` and `1,2,3,4,6`.
- Invalid example: `1,2,3,5,6` (no four-in-a-row).
- Score: **30** if valid, otherwise 0.

#### Large Straight

- Requirement: five consecutive faces. The only valid sets are `{1,2,3,4,5}` and `{2,3,4,5,6}`.
- Score: **40** if valid, otherwise 0.

#### Five of a Kind

- Requirement: all five dice show the same face.
- Score: **50** if valid, otherwise 0.

This is the only category that can later enable the Five-of-a-Kind Bonus. A 0 recorded here is still a used category.

#### Chance

- No requirement.
- Score: sum of all five dice (never 0 unless the dice sum to 0, which cannot happen with faces 1–6).

---

## 8. How a category is scored (normal rules)

When the player selects an unused category and **no Joker applies**:

1. Evaluate the category’s requirement against the five current faces.
2. If it fails, record **0** in that category.
3. If it succeeds, record the score defined above.

Five of a kind:

- **does** satisfy Three of a Kind and Four of a Kind (sum of all dice = `5 × face`)
- **does** score 50 in Five of a Kind
- **does not** satisfy Full House, Small Straight, or Large Straight

The first five-of-a-kind of the game is not forced into the Five of a Kind category. The player may assign it to any unused category under normal rules (for example five 6s as Sixes for 30).

---

## 9. End-game and win condition

The game ends when all 13 categories have a recorded score (including zeros).

Final total =

- sum of Upper Section categories
- \+ 35 if that Upper sum ≥ 63
- \+ sum of Lower Section categories
- \+ 100 for each Five-of-a-Kind Bonus awarded during the game

The player “wins” by finishing with that total. This version does not compare two participants.

---

## 10. Terminal-only requirement

All interaction is text in a terminal: dice, lock state, remaining rolls, unused and used categories, current subtotals, prompts, and errors. There is no GUI.

Exact commands and layout are left to implementation, provided they stay terminal-only and do not leak rules into untestable I/O.

---

## 11. Five-of-a-Kind Bonus

A **Five-of-a-Kind Bonus** is awarded when **all** of the following are true on a turn:

1. The current five dice are five of a kind.
2. The Five of a Kind category is already filled with **50** (not unused, and not 0).

Effect:

- Add **100** points to the game total for this occurrence. Multiple bonuses may be earned on later turns (one bonus per qualifying five-of-a-kind turn).
- The player **must choose a remaining unused category immediately**.
- The player **cannot use any remaining rolls** on that turn.

If Five of a Kind was previously filled with **0**, no 100-point bonus is awarded, and Joker rules do **not** apply. Scoring uses §8 only.

If Five of a Kind is still unused, this is not a bonus situation: the player may continue rolling (if rolls remain) or score any unused category under §8, including 50 in Five of a Kind.

---

## 12. Joker rules (unambiguous rule set)

Joker rules apply **only** when a Five-of-a-Kind Bonus applies (§11): five of a kind while Five of a Kind already scores 50.

Let `F` be the face showing on all five dice (1–6). The matching Upper category is Ones for `F=1`, Twos for `F=2`, and so on.

### 12.1 Placement (forced order)

The player still selects exactly one unused category, but legal choices are restricted:

1. **Matching Upper unused:** the player **must** select that Upper category. Score it with normal Upper rules: `5 × F`. Joker scores for Full House / straights are not used on this step.
2. **Matching Upper already used, and at least one Lower category other than Five of a Kind is unused:** the player **must** select one of those unused Lower categories. Five of a Kind cannot be chosen (it is already filled). Scoring for that choice is §12.2.
3. **Matching Upper used, and every Lower category is already filled:** the player **must** select some other unused Upper category. That category scores **0** (the dice do not show that other face).

There is always at least one unused category when a turn is in progress, so this cannot deadlock.

### 12.2 Lower Section scores under Joker

When step 12.1.2 applies:

| Selected category | Score |
|---|---|
| Three of a Kind | sum of all five dice (`5 × F`) |
| Four of a Kind | sum of all five dice (`5 × F`) |
| Full House | **25** (Joker; five of a kind is treated as a valid Full House only in this case) |
| Small Straight | **30** (Joker) |
| Large Straight | **40** (Joker) |
| Chance | sum of all five dice (`5 × F`) |

Three of a Kind, Four of a Kind, and Chance do not need a special override: five of a kind already meets them. Full House and both straights **do** need the Joker; without it they would score 0 (§8).

### 12.3 What Joker does not change

- It does not reopen a used category.
- It does not award extra Upper Section Bonus by itself; Upper Bonus still depends only on the six Upper totals.
- It does not apply when Five of a Kind is unused or was filled with 0.
- It does not allow scoring Full House or straights as 25/30/40 for a five-of-a-kind **unless** §11 applies.

### 12.4 Worked examples

**A. Bonus, matching Upper free:** Five of a Kind already 50. Roll `4,4,4,4,4`. Player must take Fours for 20. Also +100 bonus. No more rolls.

**B. Bonus, matching Upper used, Lower open:** Sixes already filled. Roll `6,6,6,6,6`. Player may take Full House (25), Small Straight (30), Large Straight (40), Three/Four of a Kind (30), or Chance (30), if unused. Also +100.

**C. Bonus, only other Uppers left:** All Lower categories and Sixes are filled. Roll `6,6,6,6,6`. Player must dump into an unused Upper (Ones–Fives) for 0. Also +100.

**D. No Joker (Yahtzee was 0):** Five of a Kind already 0. Roll `3,3,3,3,3`. No +100. Full House / straights score 0 if chosen. Threes may still score 15 if unused. Player may still reroll if rolls remain.

**E. First five-of-a-kind:** Five of a Kind unused. Roll `2,2,2,2,2`. No +100, no forced stop, no Joker. Player may score 50 in Five of a Kind, 10 in Twos, 10 in Three of a Kind, or 0 in Full House, etc.

---

## 13. Assumptions

These are in force until changed in this file:

1. Single-player only; no opponent score to beat.
2. Fair six-sided dice; independent uniform rolls.
3. Upper Bonus threshold is 63 for +35.
4. Each additional qualifying five-of-a-kind (while Five of a Kind is 50) awards another +100 in that turn, in addition to the category filled that turn.
5. Joker placement follows §12 (Hasbro-style Yahtzee: forced matching Upper, then Lower with Joker straights/house, then 0 in another Upper).
6. Small Straight uses distinct consecutive faces present in the roll (duplicates do not break a run).
7. Five of a kind is not a Full House or straight except under Joker (§12).
8. The first roll of a turn always rolls all five dice.
9. Session-only play: no save/load.
10. Standard library C++ only unless a later change justifies a dependency.
11. Game logic must be callable without a terminal (for tests).

---

## 14. Open questions

Resolved in §12 for implementation, but flagged so a later spec change is explicit:

- [x] Joker vs normal Full House / straights — **Decision:** five of a kind is invalid for those categories unless a Five-of-a-Kind Bonus (Joker) applies.
- [x] Forced Upper vs free Lower — **Decision:** matching unused Upper is mandatory; Lower Joker only if that Upper is already used.
- [x] Deadlock when Lower is full — **Decision:** unused non-matching Upper scores 0.
- [x] Bonus if Five of a Kind was 0 — **Decision:** no +100 and no Joker.
- [x] Reroll with all dice locked — **Decision:** the action will be rejected and treated as an illegal move (will be implemented on phase 6)  
- [x] Must the player confirm a category after roll 3, vs auto-prompt until valid? — **Decision:** The player must confirmt a category after roll 3  
- [x] Display of running Upper Bonus — **Decision** show “35 if Upper ≥ 63” during play  
- [x] Command language — **Decision** the user will use keys to navigate the game

Still open (choose during implementation or a spec revision; do not silently invent extra mechanics):

- [ ] **Zero-roll display / seed control** for tests: production play is random; tests may inject dice. How the binary exposes that is an implementation detail.
- [ ] **Quitting mid-game:** not specified. Until defined, a mid-game abort is not a completed game and need not write a final score.

---

## 15. References

- This document: product rules and acceptance criteria
- `ROADMAP.md`: build order, phase exits
- `TASKS.md`: current implementation checklist
- Common-knowledge Yahtzee scoring (Upper 63 / +35, extra Yahtzee +100, Joker placement) used where this brief was silent; Dice-Party is still a distinct game name and this file is the authority
