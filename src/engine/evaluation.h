#pragma once

#include "../chess/board/board.h"
#include "evalConstants.h"
#include "../bit.h"

int staticEvaluation(const Board& board);

bool isEndgame(const Board& board);