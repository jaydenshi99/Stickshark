#pragma once

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define PAWN_ENDGAME "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w HAha - 0 1"
#define ROOK_V_KING "8/8/8/3k4/8/8/8/K6R w HAha - 0 1"

#define EMPTY   -1
#define WPAWN   0
#define WBISHOP 1
#define WKNIGHT 2
#define WROOK   3
#define WQUEEN  4
#define WKING   5
#define BPAWN   6
#define BBISHOP 7
#define BKNIGHT 8
#define BROOK   9
#define BQUEEN  10
#define BKING   11

#define WK_START_SQUARE 3
#define WKR_START_SQUARE 0
#define WQR_START_SQUARE 7
#define BK_START_SQUARE 59
#define BKR_START_SQUARE 56
#define BQR_START_SQUARE 63

#define WHITE_CASTLING_RIGHTS 0b11
#define BLACK_CASTLING_RIGHTS 0b1100

#define NUM_PIECES 12
#define NUM_SQUARES 64

#define MAX_MOVES 250 // Arbitrary

#define MATE 32000

#define MAX_DEPTH 50

#define BEST_MOVE_TABLE_MAX_SIZE 2000000000