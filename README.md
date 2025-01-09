# Stickshark

Stickshark is a C++ chess engine designed for high efficiency, featuring advanced move generation through bit manipulation and selects best moves utilising negamax with alpha-beta pruning and other techniques. Its modular design and use of bitboards are optimised for performance.

Project Logs: https://www.notion.so/stickshark-42a22c01dacb4826a874dbc6d1b53562?pvs=4

Note: Castling and enpassant not yet supported

## Features
- **Efficient Move Generation**: Uses bitboards for fast chessboard representation.
- **Alpha-Beta Pruning**: Optimised decision-making.
- **Heuristics**: Implements MVV-LVA for move ordering.

---

## Prerequisites

Before building the project, ensure the following dependencies are installed:

### Compiler
- GCC 10+ or Clang 10+ with C++20 support.

### Build Tools
- `make`: The Makefile requires a compatible `make` utility.
  - Install on Linux: `sudo apt install make`
  - Install on macOS: `xcode-select --install`
  - Install on Windows: Use MinGW Make or WSL.

### Additional Libraries
- None required.

---

## Installation Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/jaydenshi99/Stickshark.git
   cd Stickshark
   ```

2. Build the project:
   ```bash
   make
   ```

3. Run the program:
   ```bash
   ./bin/main
   ```

---

## Contact
For questions or feedback, feel free to contact:
- Email: jaydenshi.js@gmail.com
