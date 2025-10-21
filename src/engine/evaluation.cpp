#include "evaluation.h"

using namespace std;

static constexpr int ATTACK_UNIT_P = 3;
static constexpr int ATTACK_UNIT_N = 3;
static constexpr int ATTACK_UNIT_B = 3;
static constexpr int ATTACK_UNIT_R = 5;
static constexpr int ATTACK_UNIT_Q = 7;

// Returns eval, positive means white is doing better
int staticEvaluation(const Board& board) {
    // Threefold repetition
    if (board.numThreefoldStates > 0) {
        return 0; 
    }

    int phase = getEndgamePhase(board);
    int evalMG = staticEvaluationMG(board);
    int evalEG = staticEvaluationEG(board);

    // Material eval
    int eval = (evalMG * phase + evalEG * (MAX_PHASE - phase)) / MAX_PHASE;

    return eval;
}

int staticEvaluationMG(const Board& board) {
    // Material eval
    int eval = 0;
    for (int i = 0; i < 12; i++) {
        eval += popcount(board.pieceBitboards[i]) * materialEvaluationsMG[i];
    }

    // piece squares
    eval += board.pieceSquareEvalMG;


    return eval;
}

int staticEvaluationEG(const Board& board) {
    // Material eval
    int eval = 0;
    for (int i = 0; i < 12; i++) {
        eval += popcount(board.pieceBitboards[i]) * materialEvaluationsEG[i];
    }

    eval += mopUpEval(board, eval);

    eval += board.pieceSquareEvalEG;

    return eval;
}

int mopUpEval(const Board& board, int materialDiff) {
    const int threshold = 200;

    // no side has a significant enough advantage for mop up eval
    if (materialDiff > -threshold && materialDiff < threshold) {
        return 0;
    }

    int wKingPos = lsb(board.pieceBitboards[WKING]);
    int bKingPos = lsb(board.pieceBitboards[BKING]);

    int kingDist = manhattanDistances[wKingPos][bKingPos];
    int centralDist = materialDiff >= threshold ? centralManhattanDistances[bKingPos] : centralManhattanDistances[wKingPos]; // CMD of losing king
    
    int eval = 15 * centralDist - 5 * (14 - kingDist);

    return materialDiff >= 200 ? eval : -eval;
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