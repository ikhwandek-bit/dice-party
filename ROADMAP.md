# Roadmap

Phases are sequential except where a later phase only *polishes* earlier work. Game rules and scoring come before terminal presentation. Do not start out-of-scope work (GUI, network, multiplayer, database).

```
Phase 1 Foundation
    → Phase 2 Dice
        → Phase 3 Scoring          (can be developed in parallel with dice
                                    once dice values are a plain 5-tuple)
            → Phase 4 Game flow    (needs dice + scoring)
                → Phase 5 Special rules (needs full scorecard + turns)
                    → Phase 6 Terminal UI (needs complete rules)
                        → Phase 7 Testing and refinement (overlaps 3–6,
                                    but the exit gate is last)
```

Scoring unit tests (Phase 3 / 7) must not depend on a finished UI.

---

## Phase 1 — Project Foundation

Status: Complete

### Outcome

A C++ terminal program that builds, starts, and exits, with core data shapes sketched and game logic separated from I/O enough to grow tests later.

### Included work

- C++ project layout and build configuration (standard toolchain; no extra libraries unless justified)
- Dice, scorecard, and game-state data structures (fields only; behavior can be stubs)
- Thin terminal entry point and a loop placeholder
- Clear boundary: rules/scoring code vs print/read code

### Depends on

- None

### Risks

- Mixing I/O into scoring early makes Phase 7 expensive
- Over-building architecture beyond what SPEC.md requires

### Exit criteria

- Project builds with the chosen build system
- A running binary prints a stub message or empty frame and exits cleanly
- Data structures for five dice, 13 categories, and turn/roll counters exist
- No GUI, network, or third-party game engines

---

## Phase 2 — Dice System

Status: Complete

### Outcome

Five dice can be rolled, locked, unlocked, and limited to three rolls per turn, independently of scoring.

### Included work

- Die face values 1–6
- Random roll of unlocked dice
- Lock and unlock any subset
- Roll counter and rejection of a fourth roll
- Reset lock state at turn start (or equivalent API)

### Depends on

- Phase 1 data structures and build

### Risks

- Hidden global RNG that tests cannot control
- Lock state accidentally persisting across turns

### Exit criteria

- Given a known RNG or injected faces, rolls and locks match SPEC.md §6
- Unlocking a die causes it to change on the next roll (under a non-fixed RNG or test double)
- A turn cannot exceed three rolls (bonus forced-stop is Phase 5)

---

## Phase 3 — Scoring System

Status: Complete

### Outcome

Every category and the Upper Section Bonus can be computed from five faces with no terminal involved. Joker overrides wait for Phase 5.

### Included work

- Upper Section (Ones–Sixes)
- Upper Section Bonus (+35 if Upper sum ≥ 63)
- Three of a Kind, Four of a Kind
- Full House (five of a kind is **not** valid)
- Small Straight, Large Straight
- Five of a Kind (50 or 0)
- Chance

### Depends on

- Five face values (Phase 2 representation, not full turn flow)

### Risks

- Straight detection wrong with duplicates
- Full House incorrectly accepting five of a kind
- Upper Bonus accidentally including Lower or Yahtzee bonus points

### Exit criteria

- Each category has tests for valid, invalid, and 0-score cases, including SPEC.md examples
- Upper Bonus is 35 iff the six Upper totals sum to ≥ 63
- Scoring functions do not read stdin or write stdout

---

## Phase 4 — Game Flow

Status: In Development

### Outcome

A 13-turn game that fills each category once, allows 0-score dumps, and reports a correct final total (without Five-of-a-Kind Bonus / Joker).

### Included work

- 13-turn structure
- Category selection from unused categories only
- Zero-score selection
- End of game when 13 categories are filled
- Final total = Upper + Upper Bonus + Lower

### Depends on

- Phase 2 (rolls/locks) and Phase 3 (scores)

### Risks

- Allowing a second write to the same category
- Ending the game early or allowing a 14th turn
- Forgetting Upper Bonus in the displayed total

### Exit criteria

- Cannot select a used category
- After 13 fills, the game stops and prints a total consistent with SPEC.md §9 excluding §11 bonuses
- Player can end a turn after roll 1, 2, or 3

---

## Phase 5 — Special Rules

Status: Not started

### Outcome

Five-of-a-Kind Bonus and Joker placement match SPEC.md §11–§12.

### Included work

- +100 when five of a kind occurs and Five of a Kind already holds 50
- No bonus and no Joker if Five of a Kind holds 0
- Immediate category choice; remaining rolls forfeited
- Forced matching Upper if unused
- Lower Joker table (Full House 25, Small Straight 30, Large Straight 40)
- Fallback: 0 in another Upper when Lower is full

### Depends on

- Phase 4 scorecard and turn engine

### Risks

- Applying Joker to the first five-of-a-kind
- Allowing Lower choice while matching Upper is still open
- Awarding +100 when Five of a Kind was 0

### Exit criteria

- SPEC.md §12.4 examples A–E behave as written
- Multiple bonuses can accrue on later turns
- Forced-stop prevents a further roll after a bonus five-of-a-kind

---

## Phase 6 — Terminal UI

Status: Not started

### Outcome

A usable command-line session that shows dice, locks, categories, scores, turn/roll, and rejects bad input without crashing.

### Included work

- Dice display including lock/unlock
- Unused vs filled categories and current totals
- Turn number and rolls used / remaining
- Input validation and error messages
- Presentation only; no new rules

### Depends on

- Phases 4–5 complete enough to play a full legal game

### Risks

- Encoding rules only in print/parse paths (breaks tests)
- Unclear errors that look like rule bugs

### Exit criteria

- A player can finish 13 turns using only the terminal
- Invalid input is rejected and the game state is unchanged
- Display matches actual state (locks, used categories, totals)

---

## Phase 7 — Testing and Refinement

Status: Not started

### Outcome

Automated coverage of rules plus a playable UX; bugs found in play are fixed against SPEC.md.

### Included work

- Unit tests: each category, bonus, Joker, dice lock/roll limits
- Tests for turn progression and duplicate category rejection
- Invalid input tests at the I/O boundary
- At least one scripted full 13-turn game with a known total
- Bugfixes and small UX clarity (not new features)

### Depends on

- Phases 2–6 (tests for scoring can start as soon as Phase 3 exists)

### Risks

- Tests that only drive the UI and miss Joker edge cases
- “Fixing” bugs by changing SPEC.md without an explicit decision

### Exit criteria

- Scoring, Joker, bonus, and 13-turn tests pass
- A complete game can be played in the terminal without a crash
- Open questions in SPEC.md that were decided in code are written back into SPEC.md
