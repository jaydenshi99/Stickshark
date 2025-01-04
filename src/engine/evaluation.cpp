#include "evaluation.h"

using namespace std;

int evaluateBoard(const Board& board) {
    int eval = 0;
    for (int i = 0; i < 12; i++) {
        eval += popcount(board.pieceBitboards[i]) * materialEvaluations[i];
    }

    return board.turn ? eval : -eval;
}