# dice-party

Terminal Yahtzee-style game in C++. See `SPEC.md`.

Build:

```text
cmake -S . -B build
cmake --build build
```

Run the game:

```text
./build/dice-party
```

Keys: `R` roll, `1`–`5` lock/unlock a die, `S` scoring tab, `0`–`12` choose category, `Q` quit.

On Windows the binary is `build\dice-party.exe` or `build\Debug\dice-party.exe`.