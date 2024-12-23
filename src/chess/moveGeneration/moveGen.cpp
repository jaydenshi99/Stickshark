#include "moveGen.h"
#include "../../utility.h"

#define FILEA 0x8080808080808080
#define FILEH 0x0101010101010101

#define RANK4 0x00FF000000
#define RANK5 0xFF00000000

using namespace std;

MoveGen::MoveGen() {};

void MoveGen::generatePawnMoves(Board b) {
    uint64_t pawnBitboard = b.turn ? b.pieceBitboards[WPAWN] : b.pieceBitboards[BPAWN];
    uint64_t doublePushRank = b.turn ? RANK4 : RANK5;
    uint64_t blockers = b.getBlockers();
    uint64_t enemy = b.turn ? b.getBlackPositions() : b.getWhitePositions();

    uint64_t singlePushes = b.turn 
        ? (pawnBitboard << 8) & ~blockers
        : (pawnBitboard >> 8) & ~blockers;

    uint64_t doublePushes = b.turn 
        ? (singlePushes << 8) & ~blockers & doublePushRank
        : (singlePushes >> 8) & ~blockers & doublePushRank;

    uint64_t leftDiagonalAttacks = b.turn
        ? (pawnBitboard << 9) & enemy & ~FILEH
        : (pawnBitboard >> 7) & enemy & ~FILEH;

    uint64_t rightDiagonalAttacks = b.turn
        ? (pawnBitboard << 7) & enemy & ~FILEA
        : (pawnBitboard >> 9) & enemy & ~FILEA;
    
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

    int leftDiagonalAttackOffset = b.turn ? -9 : 7;
    while (leftDiagonalAttacks) {
        int target = popLSB(&leftDiagonalAttacks);
        moves.push_back(Move(target + leftDiagonalAttackOffset, target, NONE));
    }

    int rightDiagonalAttackOffset = b.turn ? -7 : 9;
    while (rightDiagonalAttacks) {
        int target = popLSB(&rightDiagonalAttacks);
        moves.push_back(Move(target + rightDiagonalAttackOffset, target, NONE));
    }
}

void MoveGen::clearMoves() {
    moves.clear();
}