#include "moveGen.h"

using namespace std;

MoveGen& MoveGen::getInstance() {
    static MoveGen instance;
    return instance;
}

MoveGen::MoveGen() {
    pseudoMovesPool = new Pool(sizeof(Move) * MOVES_PER_CHUNK, MAX_POOL_CAPACITY);
    legalMovesPool = new Pool(sizeof(Move) * MOVES_PER_CHUNK, MAX_POOL_CAPACITY);
};

MoveGen::~MoveGen() {
    delete pseudoMovesPool;
    delete legalMovesPool;
}

MoveList MoveGen::generatePseudoMoves(const Board& b, bool onlyGenerateForcing) {
    this->onlyGenerateForcing = onlyGenerateForcing;

    Move* start = static_cast<Move*>(pseudoMovesPool->alloc());
    if (!start) {
        throw std::runtime_error("Failed to allocate memory for pseudo moves");
    }

    Move* curr = start;

    friendly = b.turn ? b.getWhitePositions() : b.getBlackPositions();
    enemyOrEmpty = ~friendly;

    generatePawnMoves(b, curr);
    generateKnightMoves(b, curr);
    generateSlidingMoves(b, curr);
    generateKingMoves(b, curr);

    std::ptrdiff_t count = curr - start;
    return { start, count };
}

MoveList MoveGen::generateLegalMoves(Board& b) {
    MoveList pseudoMoves = generatePseudoMoves(b, false);
    Move* start = static_cast<Move*>(legalMovesPool->alloc());
    if (!start) {
        throw std::runtime_error("Failed to allocate memory for legal moves");
    }
    Move* curr = start;

    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &move = pseudoMoves.moves[i];

        b.makeMove(move);
        if (!b.kingInCheck(false)) {
            new (curr++) Move(move);
        }
        b.unmakeMove(move);
    }

    pseudoMovesPool->free(pseudoMoves.moves);

    std::ptrdiff_t count = curr - start;
    return { start, count };
}

void MoveGen::generatePawnMoves(const Board& b, Move* &pseudoMoves) {
    uint64_t pawnBitboard = b.turn ? b.pieceBitboards[WPAWN] : b.pieceBitboards[BPAWN];
    uint64_t doublePushRank = b.turn ? rankBitboards[3] : rankBitboards[4];
    uint64_t enemy = b.turn ? b.getBlackPositions() : b.getWhitePositions();
    uint64_t empty = ~b.blockers;

    // Non forcing moves
    if (!onlyGenerateForcing) {
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
        int singlePushOffset = b.turn ? -8 : 8;
        while (singlePushes) {
            int target = popLSB(singlePushes);
            new (pseudoMoves++) Move(target + singlePushOffset, target, NONE);
        }

        while (promotions) {
            int target = popLSB(promotions);
            new (pseudoMoves++) Move(target + singlePushOffset, target, PROMOTEBISHOP);
            new (pseudoMoves++) Move(target + singlePushOffset, target, PROMOTEKNIGHT);
            new (pseudoMoves++) Move(target + singlePushOffset, target, PROMOTEROOK);
            new (pseudoMoves++) Move(target + singlePushOffset, target, PROMOTEQUEEN);
        }

        int doublePushOffset = b.turn ? -16 : 16;
        while (doublePushes) {
            int target = popLSB(doublePushes);
            new (pseudoMoves++) Move(target + doublePushOffset, target, PAWNTWOFORWARD);
        }
    }

    // Generate pawn attacks and promotions
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


    int leftDiagonalAttackOffset = b.turn ? -9 : 7;
    while (leftDiagonalAttacks) {
        int target = popLSB(leftDiagonalAttacks);
        new (pseudoMoves++) Move(target + leftDiagonalAttackOffset, target, NONE);
    }

    while (leftDiagonalAttackPromotions) {
        int target = popLSB(leftDiagonalAttackPromotions);
        new (pseudoMoves++) Move(target + leftDiagonalAttackOffset, target, PROMOTEBISHOP);
        new (pseudoMoves++) Move(target + leftDiagonalAttackOffset, target, PROMOTEKNIGHT);
        new (pseudoMoves++) Move(target + leftDiagonalAttackOffset, target, PROMOTEROOK);
        new (pseudoMoves++) Move(target + leftDiagonalAttackOffset, target, PROMOTEQUEEN);
    }

    int rightDiagonalAttackOffset = b.turn ? -7 : 9;
    while (rightDiagonalAttacks) {
        int target = popLSB(rightDiagonalAttacks);
        new (pseudoMoves++) Move(target + rightDiagonalAttackOffset, target, NONE);
    }

    while (rightDiagonalAttackPromotions) {
        int target = popLSB(rightDiagonalAttackPromotions);
        new (pseudoMoves++) Move(target + rightDiagonalAttackOffset, target, PROMOTEBISHOP);
        new (pseudoMoves++) Move(target + rightDiagonalAttackOffset, target, PROMOTEKNIGHT);
        new (pseudoMoves++) Move(target + rightDiagonalAttackOffset, target, PROMOTEROOK);
        new (pseudoMoves++) Move(target + rightDiagonalAttackOffset, target, PROMOTEQUEEN);
    }

    // enpassant
    if (b.history.top().enpassantColumn != -1) {
        int enpassantColumn = b.history.top().enpassantColumn;
        int targetSquare = b.turn ? enpassantColumn + 40 : enpassantColumn + 16;
        if (empty & (1ULL << targetSquare)) {
            uint64_t leftAttack = b.turn ? (enpassantColumn > 0 ? (1ULL << (targetSquare - 9)) : 0ULL) : (enpassantColumn > 0 ? (1ULL << (targetSquare + 7)) : 0ULL);
            uint64_t rightAttack = b.turn ? (enpassantColumn < 7 ? (1ULL << (targetSquare - 7)) : 0ULL) : (enpassantColumn < 7 ? (1ULL << (targetSquare + 9)) : 0ULL);
            uint64_t enpassantPawns = pawnBitboard & (leftAttack | rightAttack);
            while (enpassantPawns) {
                int source = popLSB(enpassantPawns);
                new (pseudoMoves++) Move(source, targetSquare, ENPASSANT);
            }
        }
    }
}

void MoveGen::generateKnightMoves(const Board& b, Move* &pseudoMoves) {
    uint64_t knightBitboard = b.turn ? b.pieceBitboards[WKNIGHT] : b.pieceBitboards[BKNIGHT];
    uint64_t enemyOrEmpty = b.turn ? ~b.getWhitePositions() : ~b.getBlackPositions();
    uint64_t forcingMask = onlyGenerateForcing ? b.blockers : ~0ULL;

    while (knightBitboard) {
        int source = popLSB(knightBitboard);
        uint64_t movesBitboard = knightAttackBitboards[source] & enemyOrEmpty & forcingMask;

        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            new (pseudoMoves++) Move(source, target, NONE);
        }
    }
}

void MoveGen::generateSlidingMoves(const Board& b, Move* &pseudoMoves) {
    uint64_t bishopBitboard = b.turn ? b.pieceBitboards[WBISHOP] : b.pieceBitboards[BBISHOP];
    uint64_t rookBitboard = b.turn ? b.pieceBitboards[WROOK] : b.pieceBitboards[BROOK];
    uint64_t queenBitboard = b.turn ? b.pieceBitboards[WQUEEN] : b.pieceBitboards[BQUEEN];
    uint64_t forcingMask = onlyGenerateForcing ? b.blockers : ~0ULL;

    // Bishop moves
    while (bishopBitboard) {
        int source = popLSB(bishopBitboard);

        uint64_t occupancy = bishopAttackMagicMasks[source] & b.blockers;
        int index = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * occupancy) >> 55);
        uint64_t movesBitboard = bishopAttackBitboards[index] & ~friendly & forcingMask;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            new (pseudoMoves++) Move(source, target, NONE);
        }
    }

    // Rook moves
    while (rookBitboard) {
        int source = popLSB(rookBitboard);

        uint64_t occupancy = rookAttackMagicMasks[source] & b.blockers;
        int index = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * occupancy) >> 52);
        uint64_t movesBitboard = rookAttackBitboards[index] & ~friendly & forcingMask;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            new (pseudoMoves++) Move(source, target, NONE);
        }
    }

    // Queen moves
    while (queenBitboard) {
        int source = popLSB(queenBitboard);

        uint64_t bishopOccupancy = bishopAttackMagicMasks[source] & b.blockers;
        int bishopIndex = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * bishopOccupancy) >> 55);

        uint64_t rookOccupancy = rookAttackMagicMasks[source] & b.blockers;
        int rookIndex = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * rookOccupancy) >> 52);

        uint64_t movesBitboard = (bishopAttackBitboards[bishopIndex] | rookAttackBitboards[rookIndex]) & ~friendly & forcingMask;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            new (pseudoMoves++) Move(source, target, NONE);
        }
    }
}

void MoveGen::generateKingMoves(const Board& b, Move* &pseudoMoves) {
    uint64_t kingBitboard = b.turn ? b.pieceBitboards[WKING] : b.pieceBitboards[BKING];
    uint64_t forcingMask = onlyGenerateForcing ? b.blockers : ~0ULL;

    while (kingBitboard) {
        int source = popLSB(kingBitboard);
        uint64_t movesBitboard = kingAttackBitboards[source] & enemyOrEmpty & forcingMask;

        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            new (pseudoMoves++) Move(source, target, NONE);
        }
    }

    if (onlyGenerateForcing) return; // no castling moves in forcing mode

    const uint64_t blockers = b.blockers;
    const uint64_t attackedByOpp = b.turn ? b.getBlackAttacks() : b.getWhiteAttacks();

    constexpr uint8_t W_K = 1u;
    constexpr uint8_t W_Q = 2u;
    constexpr uint8_t B_K = 4u;
    constexpr uint8_t B_Q = 8u;

    constexpr int E1 = WK_START_SQUARE;
    constexpr int E8 = BK_START_SQUARE;

    constexpr int G1 = 1, C1 = 5;
    constexpr int G8 = 57, C8 = 61;

    constexpr uint64_t W_K_EMPTY_MASK = 0x0000000000000006ULL;
    constexpr uint64_t W_K_SAFE_MASK  = 0x000000000000000EULL;
    constexpr uint64_t W_Q_EMPTY_MASK = 0x0000000000000070ULL;
    constexpr uint64_t W_Q_SAFE_MASK  = 0x0000000000000038ULL;
    constexpr uint64_t B_K_EMPTY_MASK = 0x0600000000000000ULL;
    constexpr uint64_t B_K_SAFE_MASK  = 0x0E00000000000000ULL;
    constexpr uint64_t B_Q_EMPTY_MASK = 0x7000000000000000ULL;
    constexpr uint64_t B_Q_SAFE_MASK  = 0x3800000000000000ULL;

    if (b.turn) {
        // White to move
        if ((b.history.top().castlingRights & W_K) &&
            !(blockers & W_K_EMPTY_MASK) &&
            !(attackedByOpp & W_K_SAFE_MASK)) {
            new (pseudoMoves++) Move(E1, G1, CASTLING);
        }

        if ((b.history.top().castlingRights & W_Q) &&
            !(blockers & W_Q_EMPTY_MASK) &&
            !(attackedByOpp & W_Q_SAFE_MASK)) {
            new (pseudoMoves++) Move(E1, C1, CASTLING);
        }
    } else {
        // Black to move
        if ((b.history.top().castlingRights & B_K) &&
            !(blockers & B_K_EMPTY_MASK) &&
            !(attackedByOpp & B_K_SAFE_MASK)) {
            new (pseudoMoves++) Move(E8, G8, CASTLING);
        }
        if ((b.history.top().castlingRights & B_Q) &&
            !(blockers & B_Q_EMPTY_MASK) &&
            !(attackedByOpp & B_Q_SAFE_MASK)) {
            new (pseudoMoves++) Move(E8, C8, CASTLING);
        }
    }
}



void MoveGen::orderMoves(Board& b, MoveList& pseudoMoves, uint16_t bestMoveValue, uint32_t killers, int killerHistory[2][64][64]) {
    uint16_t killer1 = killers & 0xFFFF;
    uint16_t killer2 = killers >> 16;

    // Assign score to each move
    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &move = pseudoMoves.moves[i];
        int attackedPiece = b.squares[move.getTarget()];

        // Big bonus if the move is the best move from pervious depths. Guaruntees that the move will be searched first.
        if (move.moveValue == bestMoveValue) {
            move.moveScore = 1000000;
        }

        // Then we search good captures. Ordered with MVV - LVA heuristic
        else if (attackedPiece != EMPTY) {
            int movedPiece = b.squares[move.getSource()];
            move.moveScore = moveScoreMaterialEvaluations[attackedPiece] - moveScoreMaterialEvaluations[movedPiece] + 900000;
        }

        // then search killer 1
        else if (move.moveValue == killer1) {
            move.moveScore = 800000;
        }

        // then search killer 2
        else if (move.moveValue == killer2) {
            move.moveScore = 790000;
        }

        // use history heuristic to score remaining non captures
        else {
            move.moveScore = killerHistory[!b.turn][move.getSource()][move.getTarget()] / (MAX_HISTORY / 100);
        }
    }

    // Sort the moves in order of descending moveScore
    sort(
        pseudoMoves.moves,
        pseudoMoves.moves + pseudoMoves.count,
        [](const Move &a, const Move &b) {
            return a.moveScore > b.moveScore;
        }
    );
}

void MoveGen::freePseudoMoves(MoveList& pseudoMoves) {
    pseudoMovesPool->free(pseudoMoves.moves);
    pseudoMoves.moves = nullptr;
    pseudoMoves.count = 0;
}

void MoveGen::freeLegalMoves(MoveList& legalMoves) {
    legalMovesPool->free(legalMoves.moves);
    legalMoves.moves = nullptr;
    legalMoves.count = 0;
}

bool MoveGen::findLegalMoveToTarget(Board& b, int targetSquare, MoveList& pseudoMoves, Move& out) {
    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &move = pseudoMoves.moves[i];
        // only care about attacks on the target square
        if (move.getTarget() != targetSquare) {
            continue;
        }

        b.makeMove(move);
        if (b.kingInCheck(false)) {
            b.unmakeMove(move);
            continue;
        }
        b.unmakeMove(move);

        // is legal move
        out = move;
        return true;
    }

    return false;
}

/**
 * Finds the least valuable attack to some target square
 */
// Move MoveGen::getLeastValuableAttack(Board& b, int targetSquare) {
//     Move* start = static_cast<Move*>(pseudoMovesPool->alloc());
//     if (!start) {
//         throw std::runtime_error("Failed to allocate memory for pseudo moves");
//     }

//     Move* pieceStart = start;
//     Move* curr = start;

//     onlyGenerateForcing = true;

//     Move leastValuableAttack = Move();

//     generatePawnMoves(b, curr);

//     if (findLegalMoveToTarget(b, targetSquare, leastValuableAttack)) {
//         return leastValuableAttack;
//     } else {
//         pseudoMoves.clear();
//     }

//     generateKnightMoves(b);

//     if (findLegalMoveToTarget(b, targetSquare, leastValuableAttack)) {
//         return leastValuableAttack;
//     } else {
//         pseudoMoves.clear();
//     }

//     generateSlidingMoves(b);
    
//     if (findLegalMoveToTarget(b, targetSquare, leastValuableAttack)) {
//         return leastValuableAttack;
//     } else {
//         pseudoMoves.clear();
//     }

//     generateKingMoves(b);
    
//     if (findLegalMoveToTarget(b, targetSquare, leastValuableAttack)) {
//         return leastValuableAttack;
//     } else {
//         pseudoMoves.clear();
//     }

//     return leastValuableAttack;
// }