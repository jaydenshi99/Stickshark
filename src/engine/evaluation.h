#pragma once

#include "../chess/board/board.h"
#include "evalConstants.h"
#include "../bit.h"

static const int PHASE_P=0, PHASE_N=1, PHASE_B=1, PHASE_R=2, PHASE_Q=4;
static const int MAX_PHASE = 24;

int staticEvaluation(const Board& board);

int getEndgamePhase(const Board& board);