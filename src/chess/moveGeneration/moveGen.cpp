#include "moveGen.h"

using namespace std;

MoveGen::MoveGen() {
    moves.reserve(MAX_MOVES);
};

void MoveGen::generatePseudoMoves(const Board& b) {
    clearMoves();

    friendly = b.turn ? b.getWhitePositions() : b.getBlackPositions();
    enemyOrEmpty = ~friendly;

    generatePawnMoves(b);
    generateKnightMoves(b);
    generateSlidingMoves(b);
    generateKingMoves(b);
}

void MoveGen::generatePawnMoves(const Board& b) {
    uint64_t pawnBitboard = b.turn ? b.pieceBitboards[WPAWN] : b.pieceBitboards[BPAWN];
    uint64_t doublePushRank = b.turn ? rankBitboards[3] : rankBitboards[4];
    uint64_t enemy = b.turn ? b.getBlackPositions() : b.getWhitePositions();
    uint64_t empty = ~b.blockers;

    // Not including pawns which promote on push
    uint64_t singlePushes = b.turn 
        ? (pawnBitboard << 8) & empty & notRankBitboards[7]
        : (pawnBitboard >> 8) & empty & notRankBitboards[0];

    uint64_t promotions = b.turn 
        ? (pawnBitboard << 8) & empty & rankBitboards[7]
        : (pawnBitboard >> 8) & empty & rankBitboards[0];

    uint64_t doublePushes = b.turn 
        ? (singlePushes << 8) & empty & doublePushRank
        : (singlePushes >> 8) & empty & doublePushRank;

    uint64_t leftDiagonalAttacks = b.turn
        ? (pawnBitboard << 9) & enemy & notFileBitboards[7] & notRankBitboards[7]
        : (pawnBitboard >> 7) & enemy & notFileBitboards[7] & notRankBitboards[0];

    uint64_t rightDiagonalAttacks = b.turn
        ? (pawnBitboard << 7) & enemy & notFileBitboards[0] & notRankBitboards[7]
        : (pawnBitboard >> 9) & enemy & notFileBitboards[0] & notRankBitboards[0];

    uint64_t leftDiagonalAttackPromotions = b.turn
        ? (pawnBitboard << 9) & enemy & notFileBitboards[7] & rankBitboards[7]
        : (pawnBitboard >> 7) & enemy & notFileBitboards[7] & rankBitboards[0];

    uint64_t rightDiagonalAttackPromotions = b.turn
        ? (pawnBitboard << 7) & enemy & notFileBitboards[0] & rankBitboards[7]
        : (pawnBitboard >> 9) & enemy & notFileBitboards[0] & rankBitboards[0];

    
    int singlePushOffset = b.turn ? -8 : 8;
    while (singlePushes) {
        int target = popLSB(singlePushes);
        moves.emplace_back(Move(target + singlePushOffset, target, NONE));
    }

    while (promotions) {
        int target = popLSB(promotions);
        moves.emplace_back(Move(target + singlePushOffset, target, PROMOTEBISHOP));
        moves.emplace_back(Move(target + singlePushOffset, target, PROMOTEKNIGHT));
        moves.emplace_back(Move(target + singlePushOffset, target, PROMOTEROOK));
        moves.emplace_back(Move(target + singlePushOffset, target, PROMOTEQUEEN));
    }

    int doublePushOffset = b.turn ? -16 : 16;
    while (doublePushes) {
        int target = popLSB(doublePushes);
        moves.emplace_back(Move(target + doublePushOffset, target, PAWNTWOFORWARD));
    }

    int leftDiagonalAttackOffset = b.turn ? -9 : 7;
    while (leftDiagonalAttacks) {
        int target = popLSB(leftDiagonalAttacks);
        moves.emplace_back(Move(target + leftDiagonalAttackOffset, target, NONE));
    }

    while (leftDiagonalAttackPromotions) {
        int target = popLSB(leftDiagonalAttackPromotions);
        moves.emplace_back(Move(target + leftDiagonalAttackOffset, target, PROMOTEBISHOP));
        moves.emplace_back(Move(target + leftDiagonalAttackOffset, target, PROMOTEKNIGHT));
        moves.emplace_back(Move(target + leftDiagonalAttackOffset, target, PROMOTEROOK));
        moves.emplace_back(Move(target + leftDiagonalAttackOffset, target, PROMOTEQUEEN));
    }

    int rightDiagonalAttackOffset = b.turn ? -7 : 9;
    while (rightDiagonalAttacks) {
        int target = popLSB(rightDiagonalAttacks);
        moves.emplace_back(Move(target + rightDiagonalAttackOffset, target, NONE));
    }

    while (rightDiagonalAttackPromotions) {
        int target = popLSB(rightDiagonalAttackPromotions);
        moves.emplace_back(Move(target + rightDiagonalAttackOffset, target, PROMOTEBISHOP));
        moves.emplace_back(Move(target + rightDiagonalAttackOffset, target, PROMOTEKNIGHT));
        moves.emplace_back(Move(target + rightDiagonalAttackOffset, target, PROMOTEROOK));
        moves.emplace_back(Move(target + rightDiagonalAttackOffset, target, PROMOTEQUEEN));
    }

    
}

void MoveGen::generateKnightMoves(const Board& b) {
    uint64_t knightBitboard = b.turn ? b.pieceBitboards[WKNIGHT] : b.pieceBitboards[BKNIGHT];
    uint64_t enemyOrEmpty = b.turn ? ~b.getWhitePositions() : ~b.getBlackPositions();

    while (knightBitboard) {
        int source = popLSB(knightBitboard);
        uint64_t movesBitboard = knightAttackBitboards[source] & enemyOrEmpty;

        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            moves.emplace_back(Move(source, target, NONE));
        }
    }
}

void MoveGen::generateSlidingMoves(const Board& b) {
    uint64_t bishopBitboard = b.turn ? b.pieceBitboards[WBISHOP] : b.pieceBitboards[BBISHOP];
    uint64_t rookBitboard = b.turn ? b.pieceBitboards[WROOK] : b.pieceBitboards[BROOK];
    uint64_t queenBitboard = b.turn ? b.pieceBitboards[WQUEEN] : b.pieceBitboards[BQUEEN];

    // Bishop moves
    while (bishopBitboard) {
        int source = popLSB(bishopBitboard);

        uint64_t occupancy = bishopAttackMagicMasks[source] & b.blockers;
        int index = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * occupancy) >> 55);
        uint64_t movesBitboard = bishopAttackBitboards[index] & ~friendly;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            moves.emplace_back(Move(source, target, NONE));
        }
    }

    // Rook moves
    while (rookBitboard) {
        int source = popLSB(rookBitboard);

        uint64_t occupancy = rookAttackMagicMasks[source] & b.blockers;
        int index = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * occupancy) >> 52);
        uint64_t movesBitboard = rookAttackBitboards[index] & ~friendly;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            moves.emplace_back(Move(source, target, NONE));
        }
    }

    // Queen moves
    while (queenBitboard) {
        int source = popLSB(queenBitboard);

        uint64_t bishopOccupancy = bishopAttackMagicMasks[source] & b.blockers;
        int bishopIndex = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * bishopOccupancy) >> 55);

        uint64_t rookOccupancy = rookAttackMagicMasks[source] & b.blockers;
        int rookIndex = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * rookOccupancy) >> 52);

        uint64_t movesBitboard = (bishopAttackBitboards[bishopIndex] | rookAttackBitboards[rookIndex]) & ~friendly;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            moves.emplace_back(Move(source, target, NONE));
        }
    }
}

void MoveGen::generateKingMoves(const Board& b) {
    uint64_t kingBitboard = b.turn ? b.pieceBitboards[WKING] : b.pieceBitboards[BKING];

    while (kingBitboard) {
        int source = popLSB(kingBitboard);
        uint64_t movesBitboard = kingAttackBitboards[source] & enemyOrEmpty;

        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            moves.emplace_back(Move(source, target, NONE));
        }
    }
}

void MoveGen::orderMoves(const Board& b, uint16_t bestMoveValue) {
    // Assign score to each move
    for (Move& move : moves) {
        int attackedPiece = b.squares[move.getTarget()];

        // Big bonus if the move is the best move from pervious depths. Guaruntees that the move will be searched first.
        if (move.moveValue == bestMoveValue) {
            move.moveScore = 100000;
        }

        // If attacked calcluate move score using MVV - LVA heuristic otherwise keep move score as 0
        // Higher score, better move is and thus should be searched earlier
        else if (attackedPiece != EMPTY) {
            int movedPiece = b.squares[move.getSource()];
            move.moveScore = moveScoreMaterialEvaluations[attackedPiece] - moveScoreMaterialEvaluations[movedPiece] + ATTACK_MODIFIER;
        }
    }

    // Sort the moves in order of descending moveScore
    sort(moves.begin(), moves.end(), [](const Move &a, const Move &b) {
        return a.moveScore > b.moveScore; 
    });

    // b.displayBoard();
    // for (Move move : moves) {
    //     cout << move << " | " << move.moveScore << " | " << bestMoveValue << endl;
    // }
}

void MoveGen::clearMoves() {
    moves.clear();
}