#pragma once

#include "../chess/board/board.h"
#include "evalConstants.h"
#include "../bit.h"
#include "../chess/moveGeneration/moveGen.h"

static const int PHASE_P=0, PHASE_N=1, PHASE_B=1, PHASE_R=2, PHASE_Q=4;
static const int MAX_PHASE = 24;

int staticEvaluation(const Board& board);

int staticEvaluationMG(const Board& board);
int staticEvaluationEG(const Board& board);

int mopUpEval(const Board& board, int materialDiff);

int pawnShieldEval(const Board& board);
int kingZoneEval(const Board& board);

int getEndgamePhase(const Board& board);

int passedPawnEval(const Board& board, int phase);
int doubledPawnEval(const Board& board, int phase);

// int staticExchangeEval(Board& board, const Move& move);
// int recursiveSEE(Board& board, int targetSquare);