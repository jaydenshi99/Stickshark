#include "evaluation.h"

using namespace std;

// Returns eval, positive means white is doing better
int staticEvaluation(const Board& board) {
    // Threefold repetition
    if (board.numThreefoldStates > 0) {
        return 0; 
    }
    
    // Material eval
    int eval = 0;
    for (int i = 0; i < 12; i++) {
        eval += popcount(board.pieceBitboards[i]) * materialEvaluationsMG[i];
    }

    eval += board.pieceSquareEvalMG;

    return eval;
}

int getEndgamePhase(const Board& board) {
    int phase = 0;
    phase += popcount(board.pieceBitboards[WPAWN] | board.pieceBitboards[BPAWN]) * PHASE_P;
    phase += popcount(board.pieceBitboards[WKNIGHT] | board.pieceBitboards[BKNIGHT]) * PHASE_N;
    phase += popcount(board.pieceBitboards[WBISHOP] | board.pieceBitboards[BBISHOP]) * PHASE_B;
    phase += popcount(board.pieceBitboards[WROOK] | board.pieceBitboards[BROOK]) * PHASE_R;
    phase += popcount(board.pieceBitboards[WQUEEN] | board.pieceBitboards[BQUEEN]) * PHASE_Q;

    phase = min(phase, MAX_PHASE);
    return phase;
}