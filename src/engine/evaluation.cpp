#include "evaluation.h"

using namespace std;

// Returns eval, positive means white is doing better
int staticEvaluation(const Board& board) {
    // Threefold repetition
    if (board.numThreefoldStates > 0) {
        return 0; 
    }
    
    int eval = 0;
    for (int i = 0; i < 12; i++) {
        eval += popcount(board.pieceBitboards[i]) * materialEvaluations[i];
    }

    eval += board.pieceSquareEval;

    return eval;
}