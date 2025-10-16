# Stickshark

Stickshark is a C++ chess engine designed for high efficiency, featuring advanced move generation through bit manipulation and selects best moves utilising negamax with alpha-beta pruning and other techniques.

Project Logs: https://www.notion.so/stickshark-42a22c01dacb4826a874dbc6d1b53562?pvs=4

Note: Enpassant not yet supported

## Features
- **Efficient Move Generation**: Uses bitboards for fast chessboard representation.
- **Alpha-Beta Pruning**: Optimised decision-making.
- **Heuristics**: Implements MVV-LVA for move ordering.
- **User Interface**: Visual chess board accessible via web browser.


## Prerequisites

Before building the project, ensure the following dependencies are installed:

### Compiler
- GCC 10+ or Clang 10+ with C++20 support.

### Build Tools
- `make`: The Makefile requires a compatible `make` utility.

### Additional Libraries
- None required.

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


## Web Interface

Stickshark includes a web-based GUI for visual chess gameplay.

### Launching the Web Interface

1. **Build the web version** (if not already built):
   ```bash
   make
   ```

2. **Start the HTTP server**:
   ```bash
   python3 -m http.server 8000
   ```

3. **Start the chess engine in web mode** (in another terminal):
   ```bash
   ./build/bin/main --server
   ```

4. **Open your web browser** and navigate to:
   - **Visual Board**: http://localhost:8000/web/chess_board.html
   - **Raw JSON View**: http://localhost:8000/web/json_viewer.html

## Versions

### 1.1
- Added pawn shield evaluation, punishing engine for not having pawns infront of king

### 1.0 - Initial Release
- **Core Engine**: Basic chess engine with negamax search and alpha-beta pruning
- **Move Generation**: Bitboard-based move generation for all piece types
- **Evaluation**: Piece-square tables + basic material evaluation + endgame bonus
- **Web Interface**: HTTP server with visual chess board
- **Features**: 
  - Legal move validation
  - Drag-and-drop interface
  - Board flipping
  - Engine vs human gameplay

- **Transposition Table**: Added TT for caching search results (~50MB), only used for move ordering currently
- **Quiescence Search**: Extended search for captures and checks
- **Move Ordering**: MVV-LVA (Most Valuable Victim - Least Valuable Attacker) + best move heuristic
- **Threefold Repetition**: Zobrist hash-based repetition detection

## Contact
For questions or feedback, feel free to contact:
- Email: jaydenshi.js@gmail.com
