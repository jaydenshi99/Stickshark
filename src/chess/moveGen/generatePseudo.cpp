#include "generatePseudo.h"

using namespace std;

vector<Move> generatePawnMoves(Board b) {
    int singlePushShiftAmt = b.turn ? 8 : -8;
    int pawnIndex = b.turn ? WPAWN : BPAWN;
    uint64_t blockers = b.getBlockers();

    vector<Move> moves;

    uint64_t singlePushes = (b.pieceBitboards[pawnIndex] << singlePushShiftAmt) & ~(blockers);
    
    while (singlePushes) {
        int target = popLSB(&singlePushes);
        moves.push_back(Move(target - singlePushShiftAmt, target, NONE));
    }

    return moves;
}