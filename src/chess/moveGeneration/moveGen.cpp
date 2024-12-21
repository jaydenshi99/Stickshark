#include "moveGen.h"

#define RANK4 0x00FF000000
#define RANK5 0xFF00000000

using namespace std;

MoveGen::MoveGen() {};

void MoveGen::generatePawnMoves(Board b) {
    int singlePushShiftAmt = b.turn ? 8 : -8;
    int doublePushShiftAmt = b.turn ? 16 : -16;
    int pawnIndex = b.turn ? WPAWN : BPAWN;
    uint64_t doublePushRank = b.turn? RANK4 : RANK5;
    uint64_t blockers = b.getBlockers();

    uint64_t singlePushes = (b.pieceBitboards[pawnIndex] << singlePushShiftAmt) & ~(blockers);
    uint64_t doublePushes = (singlePushes << singlePushShiftAmt) & ~(blockers) & doublePushRank;
    
    while (singlePushes) {
        int target = popLSB(&singlePushes);
        moves.push_back(Move(target - singlePushShiftAmt, target, NONE));
    }

    while (doublePushes) {
        int target = popLSB(&doublePushes);
        moves.push_back(Move(target - doublePushShiftAmt, target, PAWNTWOFORWARD));
    }
}