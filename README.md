# Stickshark

Version 2.0.4 of Stickshark, a C++ chess engine. 

Play against Stickshark on [lichess](https://lichess.org/@/stickshark99)! (The bot is currently not accepting unlimited time controls.)

Currently rated ~2000 for bullet and ~2000 for blitz time controls.

## Prerequisites

Before building the project, ensure the following dependencies are installed:

### Compiler
- GCC 10+ or Clang 10+ with C++20 support.

### Build Tools
- `cmake` (3.15+)

## Build

1) Clone:
```bash
git clone https://github.com/jaydenshi99/Stickshark.git
cd Stickshark
```

2) Configure:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

3) Build:
```bash
cmake --build build
```

4) Run (UCI mode by default; it waits for UCI commands):
- macOS/Linux: `./build/stickshark`
- Windows: `build\stickshark.exe`

Clean: remove the `build` directory to force a fresh configure/build:
- macOS/Linux: `rm -rf build`
- Windows: `rmdir /s /q build`

## Web Interface

Stickshark includes a web-based GUI for visual chess gameplay.

### Launching the Web Interface

1. **Build the engine** (if not already built):
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```

2. **Start the HTTP server**:
   ```bash
   python3 -m http.server 8000
   ```

3. **Start the chess engine in web mode** (in another terminal):
   - macOS/Linux:
     ```bash
     ./build/stickshark --server
     ```
   - Windows:
     ```powershell
     build\stickshark.exe --server
     ```

4. **Open your web browser** and navigate to:
   - **Visual Board**: http://localhost:8000/web/chess_board.html
   - **Raw JSON View**: http://localhost:8000/web/json_viewer.html

## Versions

### 2.1
- 2.1.0: Add history heuristic and change move ordering logic.

### 2.0
- 2.0.5: Fix mate packing with ply. Add killer moves.
- 2.0.4: Fix null move pruning. Add PVS. 4x TT size and upgrade from single entry to 2-way.
- 2.0.3: Pawn structure evaluation: Passed, doubled and isolated pawns.
- 2.0.2: Fix three fold repetition bug with transposition tables.
- 2.0.1: Time management, and connect to lichess servers.
- 2.0.0: Add enpassant. And move generation tests.

### 1.3
- 1.3.1: Use singleton pattern in move gen to optimise memory allocation. 26Mn/s -> 28Mn/s. Compile with cmake.
- 1.3.0: Replaced hashmap repetition tracking with Zobrist history backward scan. 12Mn/s -> 26Mn/s

### 1.2
- 1.2.3: Check extensions with SEE > 0 guard
- 1.2.2: TT for quiescence search
- 1.2.1: Transposition queries for upper and lower bound
- 1.2.0: Transposition exact queries for normal search

### 1.1
- 1.1.4: Null move pruning
- 1.1.3: Tweaked parameters in king attack penalty evaluation
- 1.1.2: Fixed bug in mop up evaluation
- 1.1.1: Added penalty for attacking pieces around king
- 1.1.0: Added pawn shield evaluation, punishing engine for not having pawns infront of king

### 1.0
- Core Engine: Basic chess engine with negamax search and alpha-beta pruning
- Move Generation: Bitboard-based move generation for all piece types
- Evaluation: Piece-square tables + basic material evaluation + endgame bonus
- Web Interface: HTTP server with visual chess board
- Features: 
  - Legal move validation
  - Drag-and-drop interface
  - Board flipping
  - Engine vs human gameplay
- Transposition Table: Added TT for caching search results, only used for move ordering
- Quiescence Search: Extended search for captures and checks
- Move Ordering: MVV-LVA (Most Valuable Victim - Least Valuable Attacker) + best move heuristic
- Threefold Repetition: Zobrist hash-based repetition detection
