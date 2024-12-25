#include "moveGen.h"

using namespace std;

MoveGen::MoveGen() {
    moves.reserve(MAX_MOVES);
};

void MoveGen::generatePseudoMoves(const Board& b) {
    clearMoves();
    generatePawnMoves(b);
    generateKnightMoves(b);
    generateSlidingMoves(b);
}

void MoveGen::generatePawnMoves(const Board& b) {
    uint64_t pawnBitboard = b.turn ? b.pieceBitboards[WPAWN] : b.pieceBitboards[BPAWN];
    uint64_t doublePushRank = b.turn ? rankBitboards[3] : rankBitboards[4];
    uint64_t blockers = b.getBlockers();
    uint64_t enemy = b.turn ? b.getBlackPositions() : b.getWhitePositions();

    uint64_t singlePushes = b.turn 
        ? (pawnBitboard << 8) & ~blockers
        : (pawnBitboard >> 8) & ~blockers;

    uint64_t doublePushes = b.turn 
        ? (singlePushes << 8) & ~blockers & doublePushRank
        : (singlePushes >> 8) & ~blockers & doublePushRank;

    uint64_t leftDiagonalAttacks = b.turn
        ? (pawnBitboard << 9) & enemy & notFileBitboards[7]
        : (pawnBitboard >> 7) & enemy & notFileBitboards[7];

    uint64_t rightDiagonalAttacks = b.turn
        ? (pawnBitboard << 7) & enemy & notFileBitboards[0]
        : (pawnBitboard >> 9) & enemy & notFileBitboards[0];
    
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

void MoveGen::generateKnightMoves(const Board& b) {
    uint64_t knightBitboard = b.turn ? b.pieceBitboards[WKNIGHT] : b.pieceBitboards[BKNIGHT];
    uint64_t enemyOrEmpty = b.turn ? ~b.getWhitePositions() : ~b.getBlackPositions();

    while (knightBitboard) {
        int source = popLSB(&knightBitboard);
        uint64_t movesBitboard = knightAttackBitboards[source] & enemyOrEmpty;

        while (movesBitboard) {
            int target = popLSB(&movesBitboard);
            moves.push_back(Move(source, target, NONE));
        }
    }
}

void MoveGen::generateSlidingMoves(const Board& b) {
    uint64_t bishopBitboard = b.turn ? b.pieceBitboards[WBISHOP] : b.pieceBitboards[BBISHOP];
    uint64_t rookBitboard = b.turn ? b.pieceBitboards[WROOK] : b.pieceBitboards[BROOK];
    uint64_t queenBitboard = b.turn ? b.pieceBitboards[WQUEEN] : b.pieceBitboards[BQUEEN];
    uint64_t friendlyBitboard = b.turn ? b.getWhitePositions() : b.getBlackPositions();
    uint64_t blockers = b.getBlockers();

    // Bishop moves
    while (bishopBitboard) {
        int source = popLSB(&bishopBitboard);

        uint64_t occupancy = bishopAttackMagicMasks[source] & blockers;
        int index = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * occupancy) >> 55);
        uint64_t movesBitboard = bishopAttackBitboards[index] & ~friendlyBitboard;
        while (movesBitboard) {
            int target = popLSB(&movesBitboard);
            moves.push_back(Move(source, target, NONE));
        }
    }

    // Rook moves
    while (rookBitboard) {
        int source = popLSB(&rookBitboard);

        uint64_t occupancy = rookAttackMagicMasks[source] & blockers;
        int index = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * occupancy) >> 52);
        uint64_t movesBitboard = rookAttackBitboards[index] & ~friendlyBitboard;
        while (movesBitboard) {
            int target = popLSB(&movesBitboard);
            moves.push_back(Move(source, target, NONE));
        }
    }

    // Queen moves
    while (queenBitboard) {
        int source = popLSB(&queenBitboard);

        uint64_t bishopOccupancy = bishopAttackMagicMasks[source] & blockers;
        int bishopIndex = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * bishopOccupancy) >> 55);

        uint64_t rookOccupancy = rookAttackMagicMasks[source] & blockers;
        int rookIndex = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * rookOccupancy) >> 52);

        uint64_t movesBitboard = (bishopAttackBitboards[bishopIndex] | rookAttackBitboards[rookIndex]) & ~friendlyBitboard;
        while (movesBitboard) {
            int target = popLSB(&movesBitboard);
            moves.push_back(Move(source, target, NONE));
        }
    }
}

void MoveGen::clearMoves() {
    moves.clear();
}