#include "generatePseudo.h"

using namespace std;

vector<Move> generatePawnMoves(Board b) {
    int singlePushShiftAmt = b.turn ? 8 : -8;
    int pawnIndex = b.turn ? WPAWN : BPAWN;
    uint64_t blockers = b.getBlockers();

    vector<Move> moves;

    uint64_t singlePushes = (b.pieceBitboards[pawnIndex] << singlePushShiftAmt) & ~(blockers);
    
    while (singlePushes) {
        int lsb = __builtin_ctzll(singlePushes);
        singlePushes &= singlePushes - 1;
    }

}