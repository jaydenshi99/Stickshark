#pragma once

#include "../constants.h"

#include <cmath>

extern const int materialEvaluationsMG[NUM_PIECES];
extern const int materialEvaluationsEG[NUM_PIECES];

extern int kingZoneAttackPenalty[1000];

extern const int moveScoreMaterialEvaluations[NUM_PIECES];

extern int pieceSquareTablesMG[NUM_PIECES][NUM_SQUARES];

extern int pieceSquareTablesEG[NUM_PIECES][NUM_SQUARES];

extern const int passedPawnBonusMG[8];
extern const int passedPawnBonusEG[8];

void calculatePieceSquareTables();

void calculateKingZoneAttackPenalty();