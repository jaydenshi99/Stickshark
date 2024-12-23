#include "moveGen.h"
#include "../../utility.h"

#define RANK4 0x00FF000000
#define RANK5 0xFF00000000

using namespace std;

MoveGen::MoveGen() {};

void MoveGen::generatePawnMoves(Board b) {
    int pawnIndex = b.turn ? WPAWN : BPAWN;
    uint64_t doublePushRank = b.turn? RANK4 : RANK5;
    uint64_t blockers = b.getBlockers();

    uint64_t singlePushes = b.turn 
        ? (b.pieceBitboards[pawnIndex] << 8) & ~blockers
        : (b.pieceBitboards[pawnIndex] >> 8) & ~blockers;

    uint64_t doublePushes = b.turn 
        ? (singlePushes << 8) & ~blockers & doublePushRank
        : (singlePushes >> 8) & ~blockers & doublePushRank;
    
    int singlePushOffset = b.turn ? -8 : 8;
    while (singlePushes) {
        int target = popLSB(&singlePushes);
        moves.push_back(Move(target + singlePushOffset, target, NONE));
    }

    int doublePushOffset = b.turn ? -16 : 16;
    while (doublePushes) {
        int target = popLSB(&doublePushes);
        moves.push_back(Move(target + doublePushOffset, target, PAWNTWOFORWARD));
    }
}

void MoveGen::clearMoves() {
    moves.clear();
}