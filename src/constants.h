#pragma once

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

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

#define NUM_PIECES 12
#define NUM_SQUARES 64

#define MAX_MOVES 250 // Arbitrary

#define MAX_EVAL 999999999
#define MIN_EVAL -999999999

#define MAX_DEPTH 30

#define BEST_MOVE_TABLE_MAX_SIZE 2000000000