# Tasks

Prioritize game rules and core logic before terminal presentation. Do not add GUI, networking, multiplayer, or database work.

## Completion rule

Move a task to completed only after its acceptance criteria and the phase exit criteria in `ROADMAP.md` that it supports have passed. Keep this list aligned with `SPEC.md`; if implementation decides an open question, update `SPEC.md` in the same change.

## Completed tasks

### Phase 1 — Project Foundation

- [x] Create C++ source layout (e.g. `src/` for program, separate file(s) for rules vs I/O)
- [x] Add a build configuration that compiles a terminal executable with no extra libraries
- [x] Define a five-dice representation (faces 1–6, lock flags)
- [x] Define a scorecard representation (13 categories, unused vs filled, recorded values)
- [x] Define game-state fields (turn 1–13, rolls used this turn 1–3, running bonuses)
- [x] Add `main` that builds, starts, and exits without playing a real game
- [x] Keep scoring/state functions free of `cin`/`cout` (I/O only at the edge)

## Current tasks

Phase 1 exit criteria passed (CMake build, stub run, data types, I/O isolated). Phase 2 is next; do not start later phases until its exit criteria pass (scoring tests in Phase 3/7 may begin once dice faces exist).

---

## Phase 2 — Dice System

- [x] Roll a single unlocked die to a value in 1–6
- [x] Roll only unlocked dice; leave locked faces unchanged
- [x] Lock any subset of the five dice after a roll
- [x] Unlock previously locked dice before the next roll
- [ ] Count rolls and refuse a fourth roll in the same turn
- [ ] Reset all locks (and roll count) when a new turn starts
- [ ] Inject or seed RNG so dice behavior can be tested without the terminal

## Phase 3 — Scoring System

- [ ] Score Ones–Sixes as the sum of matching faces
- [ ] Award Upper Section Bonus 35 iff the six Upper totals sum to ≥ 63
- [ ] Score Three of a Kind (sum of five, or 0)
- [ ] Score Four of a Kind (sum of five, or 0)
- [ ] Score Full House as 25; reject five of a kind
- [ ] Score Small Straight as 30 (four consecutive faces; duplicates allowed)
- [ ] Score Large Straight as 40 only for 1–5 and 2–6
- [ ] Score Five of a Kind as 50 or 0
- [ ] Score Chance as the sum of all five dice
- [ ] Add unit tests for SPEC.md examples and the invalid cases in §7–§8 (no terminal)

## Phase 4 — Game Flow

- [ ] Start a game with 13 unused categories
- [ ] Require a first roll every turn
- [ ] Allow category selection after roll 1, 2, or 3
- [ ] Force category selection after roll 3
- [ ] Reject selecting a category that is already filled
- [ ] Allow filling a category with 0 when the dice do not match
- [ ] Advance exactly 13 turns, one category per turn
- [ ] Compute final total: Upper + Upper Bonus + Lower (no Joker yet)
- [ ] End the game after the 13th fill and report the total

## Phase 5 — Special Rules

- [ ] Detect five of a kind while Five of a Kind already holds 50
- [ ] Add 100 to the total for each such occurrence
- [ ] Forfeit remaining rolls and require an immediate category choice
- [ ] Force the matching Upper category when it is still unused (`5 × face`)
- [ ] If matching Upper is used, allow unused Lower categories only
- [ ] Joker: Full House 25, Small Straight 30, Large Straight 40
- [ ] Three of a Kind / Four of a Kind / Chance under Joker score `5 × face`
- [ ] If Lower is full, force another unused Upper for 0
- [ ] If Five of a Kind holds 0: no +100, no Joker, normal §8 scoring
- [ ] Cover SPEC.md §12.4 examples A–E with tests

## Phase 6 — Terminal UI

- [ ] Show all five faces after each roll
- [ ] Show which dice are locked vs unlocked
- [ ] Show unused and filled categories with recorded scores
- [ ] Show Upper subtotal, Upper Bonus if earned or pending threshold, Lower subtotal, grand total
- [ ] Show turn number (1–13) and rolls used / remaining
- [ ] Accept lock/unlock and category commands; reject malformed input without changing state
- [ ] Print a clear error when a category is used or a fourth roll is requested
- [ ] Print the final breakdown at game end

## Phase 7 — Testing and Refinement

- [ ] Unit-test every category, including 0-score dumps
- [ ] Test lock/unlock and the three-roll limit
- [ ] Test turn progression and duplicate-category rejection
- [ ] Test invalid terminal input (I/O adapter tests or scripted stdin)
- [ ] Test Five-of-a-Kind Bonus (including no bonus when the category is 0)
- [ ] Test Joker forced Upper, Lower table, and Upper dump-for-0
- [ ] Script at least one full 13-turn game with known dice and assert the final total
- [ ] Fix defects found in tests or play against SPEC.md
- [ ] Record any newly decided open questions in SPEC.md
