# 🐙 Cephalopods Game – Codingame Spring Challenge 2025

This repository contains my solution for the CodinGame Spring Challenge, which involved simulating the abstract two-player board game Mark Steere's Cephalopods.

## 🎯 Objective

Simulate all possible future board states of an ongoing game of Cephalopods after a given number of turns. The goal is to compute a hash sum of all possible resulting board states.

## 🧠 Game Rules

Cephalopods is played on a 3x3 grid with dice. Players alternate turns, placing one die per turn. The game ends when the board is full.

* Dice values: 1 to 6
* Empty cells: 0
* Dice do not track player ownership

## 🔁 Turn Mechanics

1. Non-Capturing Placement

    * If placing a die adjacent to less than 2 other dice, or if no valid capture is available, the die is placed with a value of 1.

2. Capturing Placement

    * If the die is placed adjacent to 2 or more dice and any combination of their values adds up to ≤ 6, the player must capture.
    * The captured dice are removed.
    * The newly placed die takes on the sum of the removed dice values.
    * If multiple capture combinations are available, all must be explored as separate valid game paths.

3. Captures are mandatory when possible.

## 🏁 End Game

* The game ends once the board is full (no empty spaces).
* The winner would be the player with the most dice showing 6 — but this scoring is irrelevant for this challenge.

## 🧮 Board State Hashing

Each board state is hashed as a 32-bit integer:
* The board is read left to right, top to bottom
* Each cell value becomes a digit
* Values are concatenated to form the hash

## 🧾 Example

```
Board:
1 0 0
3 2 1
0 1 5

Hash:
100321015
```

## 💡 C++ Code Overview
This C++ program simulates all valid game states of Mark Steere's Cephalopods after a given number of turns, and computes the sum of the hash values of all final board states, modulo 2^30.

### 🔄 Simulation Flow
1. Input Parsing
The program reads the simulation `depth` and the initial 3x3 board configuration from standard input.

2. Bit-packed Board Representation
The board is stored as a 27-bit integer (`Board.bits`), using 3 bits per cell (enough to store values from 0–6). This enables fast memory access and compact storage.

3. Move Generation
For each empty cell, the program tries all valid moves:
    * Capturing moves: If 2 or more neighboring dice can be captured (i.e., their total value is ≤ 6), the move is mandatory. All valid combinations are explored.
    * Non-capturing move: If no captures are available, a die with value 1 is placed.

4. Memoization for Performance
To avoid recalculating the same board configurations at the same depth, the program uses a custom hash table (`memoTable`) with a precomputed size based on simulation depth. This significantly improves runtime for larger depths.

5. Recursive Simulation
The `simulate` function recursively explores all move possibilities up to the specified depth or until the board is full. At each terminal state, the board is converted to a base-10 integer (as defined in the problem statement) and added to the result.

6. Final Result
The hash values of all valid end states are summed and printed modulo 2^30.

### ⚙️ Optimizations Used
* GCC Intrinsics & Compiler Hints
    * `#pragma GCC optimize("Ofast,unroll-loops,inline")` enables aggressive compiler-level optimizations.
    * Target-specific instruction sets (`avx`, `fma`, `bmi2`) improve performance on modern CPUs.

* Bitmasking
    * Fast detection and iteration of empty cells using bitmasks.

* Precomputed Neighbor & Capture Combinations
    * All potential capturing combinations for each board position are computed once at startup.

* Compact Hashing & Modulo Arithmetic
Custom 32-bit hash combined with fast modulo arithmetic ensures performance within constraints.

### 🏁 Result
✅ Final Score: 1772 ms average on final validators
🥇 Ranking: 261st place out of 2865 participants

## 💡 Rust Code Overview
The Rust implementation follows a very similar structure to the C++ solution but incorporates some specific optimizations that led to improved performance in this particular case.

Key Differences in the Rust Version:

Memory Efficiency: Rust's memory model offers more fine-grained control over ownership and borrowing, allowing the program to avoid unnecessary copies of large data structures. This can reduce the overhead of allocating and deallocating memory compared to C++ in some situations.

HashMap: The Rust version utilizes `std::collections::HashMap` to store intermediate results and count ways for each board state at each depth. While C++ uses `unordered_map` (which is also fast), Rust's `HashMap` has some optimizations in its implementation that may perform better with smaller key-value pairs in memory-bound problems, especially with the use of the `entry` API to modify values efficiently.

Rust’s Zero-Cost Abstractions: The program benefits from Rust's zero-cost abstractions, meaning that the abstraction level doesn't add any overhead during compilation. This is particularly beneficial in scenarios where fine-grained control over performance (like bitwise operations and hashing) is essential.

### 🏁 Result
✅ Final Score: 1406  ms average on final validators
🥇 Ranking: 220th place out of 2865 participants

## Why Rust Outperformed C++
The Rust implementation provided a better performance than the C++ solution in this specific case, and the reasons for this improvement are likely tied to several factors:

1. **Efficient Memory Management:** Rust’s ownership system ensures that memory is managed efficiently. It allows for more controlled and predictable memory usage, reducing the risk of memory leaks or unnecessary allocations.

2. **Faster HashMap Access:** The HashMap in Rust, especially when used with its entry API, can sometimes outperform C++'s unordered_map in scenarios where there are many updates to existing entries. This is because the entry API in Rust provides a direct way to modify values in place, avoiding some overhead from manual checking and updating in C++.

3. **Concurrency and Safety Without the Cost:** Rust’s concurrency model is designed to be safe and effective without introducing significant runtime overhead. Although this solution doesn’t involve explicit concurrency, the inherent optimizations in Rust’s safety features may contribute to more stable and predictable performance, especially in memory-bound problems like this one.

4. **Better Compiler Optimizations:** The Rust compiler (LLVM-based) is known for its strong optimization capabilities. While both C++ and Rust rely on LLVM for their backend, the Rust compiler's optimizations can sometimes be better aligned with the needs of a specific problem, especially when dealing with bit-level operations and high-frequency function calls.

# 👨‍💻 Author

* [Ali FAHS](https://github.com/fahsAli)