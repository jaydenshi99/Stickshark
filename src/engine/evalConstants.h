#pragma once

#include "../constants.h"

extern const int materialEvaluations[NUM_PIECES];

extern const int moveScoreMaterialEvaluations[NUM_PIECES];

extern int pieceSquareTablesMG[NUM_PIECES][NUM_SQUARES];

extern int pieceSquareTablesEG[NUM_PIECES][NUM_SQUARES];

void calculatePieceSquareTables();