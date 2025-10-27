#include "evaluation.h"

using namespace std;

static constexpr int ATTACK_UNIT_P = 3;
static constexpr int ATTACK_UNIT_N = 3;
static constexpr int ATTACK_UNIT_B = 3;
static constexpr int ATTACK_UNIT_R = 5;
static constexpr int ATTACK_UNIT_Q = 7;

// Returns eval, positive means white is doing better
int staticEvaluation(const Board& board) {
    int phase = getEndgamePhase(board);
    int evalMG = staticEvaluationMG(board);
    int evalEG = staticEvaluationEG(board);

    // interpolate between mg and eg
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

    // king safety
    eval += pawnShieldEval(board);
    eval += kingZoneEval(board);

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

    int eval = 21 * centralDist + 6 * (14 - kingDist);

    return materialDiff >= 200 ? eval : -eval;
}

int pawnShieldEval(const Board& board) {
    int eval = 0;
    int wKingPos = lsb(board.pieceBitboards[WKING]);
    int bKingPos = lsb(board.pieceBitboards[BKING]);

    uint64_t wKingPawnShieldMask = kingPawnShieldMasks[wKingPos];
    uint64_t bKingPawnShieldMask = kingPawnShieldMasks[bKingPos + NUM_SQUARES];

    uint64_t wPawnShield = board.pieceBitboards[WPAWN] & wKingPawnShieldMask;
    uint64_t bPawnShield = board.pieceBitboards[BPAWN] & bKingPawnShieldMask;

    // punish for missing shield pawns
    eval -= popcount(wPawnShield ^ wKingPawnShieldMask) * 12; // white
    eval += popcount(bPawnShield ^ bKingPawnShieldMask) * 12; // black

    return eval;
}

// rough guideline: 
int kingZoneEval(const Board& board) {
    int wKingPos = lsb(board.pieceBitboards[WKING]);
    int bKingPos = lsb(board.pieceBitboards[BKING]);

    uint64_t wKingZone = kingZoneMasks[wKingPos];
    uint64_t bKingZone = kingZoneMasks[bKingPos + NUM_SQUARES];

    int wKingPenalty = 0;
    int bKingPenalty = 0;

    wKingPenalty += popcount(wKingZone & board.getPieceAttacks(BPAWN)) * ATTACK_UNIT_P;
    wKingPenalty += popcount(wKingZone & board.getPieceAttacks(BBISHOP)) * ATTACK_UNIT_B;
    wKingPenalty += popcount(wKingZone & board.getPieceAttacks(BKNIGHT)) * ATTACK_UNIT_N;
    wKingPenalty += popcount(wKingZone & board.getPieceAttacks(BROOK)) * ATTACK_UNIT_R;
    wKingPenalty += popcount(wKingZone & board.getPieceAttacks(BQUEEN)) * ATTACK_UNIT_Q;

    bKingPenalty += popcount(bKingZone & board.getPieceAttacks(WPAWN)) * ATTACK_UNIT_P;
    bKingPenalty += popcount(bKingZone & board.getPieceAttacks(WBISHOP)) * ATTACK_UNIT_B;
    bKingPenalty += popcount(bKingZone & board.getPieceAttacks(WKNIGHT)) * ATTACK_UNIT_N;
    bKingPenalty += popcount(bKingZone & board.getPieceAttacks(WROOK)) * ATTACK_UNIT_R;
    bKingPenalty += popcount(bKingZone & board.getPieceAttacks(WQUEEN)) * ATTACK_UNIT_Q;

    wKingPenalty = min(wKingPenalty, 1000);
    bKingPenalty = min(bKingPenalty, 1000);
    
    return kingZoneAttackPenalty[bKingPenalty] - kingZoneAttackPenalty[wKingPenalty];
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