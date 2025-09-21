#include "moveGen.h"

using namespace std;

MoveGen::MoveGen() {
    pseudoMoves.reserve(MAX_MOVES);
    legalMoves.reserve(MAX_MOVES);
    onlyGenerateForcing = true;
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

void MoveGen::generateLegalMoves(Board& b) {
    pseudoMoves.clear();
    legalMoves.clear();

    generatePseudoMoves(b);

    for (Move &move : pseudoMoves) {
        b.makeMove(move);
        if (!b.kingInCheck()) {
            legalMoves.emplace_back(move);
        }
        b.unmakeMove(move);
    }
}

void MoveGen::generatePawnMoves(const Board& b) {
    uint64_t pawnBitboard = b.turn ? b.pieceBitboards[WPAWN] : b.pieceBitboards[BPAWN];
    uint64_t doublePushRank = b.turn ? rankBitboards[3] : rankBitboards[4];
    uint64_t enemy = b.turn ? b.getBlackPositions() : b.getWhitePositions();
    uint64_t empty = ~b.blockers;

    uint64_t forcingMask = ~0ULL;
    if (onlyGenerateForcing) {
        uint64_t enemyKingBitboard = b.turn ? b.pieceBitboards[BKING] : b.pieceBitboards[WKING];

        uint64_t pawnCheckingPositionsLeft = !b.turn
            ? (enemyKingBitboard << 9) & notFileBitboards[7] & notRankBitboards[7]
            : (enemyKingBitboard >> 7) & notFileBitboards[7] & notRankBitboards[0];

        uint64_t pawnCheckingPositionsRight = !b.turn
            ? (enemyKingBitboard << 7) & notFileBitboards[0] & notRankBitboards[7]
            : (enemyKingBitboard >> 9) & notFileBitboards[0] & notRankBitboards[0];

        forcingMask = b.blockers | pawnCheckingPositionsLeft | pawnCheckingPositionsRight;
    }

    // Not including pawns which promote on push
    uint64_t singlePushes = b.turn 
    ? (pawnBitboard << 8) & empty & notRankBitboards[7] & forcingMask
    : (pawnBitboard >> 8) & empty & notRankBitboards[0] & forcingMask;

    uint64_t promotions = b.turn 
        ? (pawnBitboard << 8) & empty & rankBitboards[7] & forcingMask
        : (pawnBitboard >> 8) & empty & rankBitboards[0] & forcingMask;

    uint64_t doublePushes = b.turn 
        ? (singlePushes << 8) & empty & doublePushRank & forcingMask
        : (singlePushes >> 8) & empty & doublePushRank & forcingMask;
    int singlePushOffset = b.turn ? -8 : 8;
    while (singlePushes) {
        int target = popLSB(singlePushes);
        pseudoMoves.emplace_back(Move(target + singlePushOffset, target, NONE));
    }

    while (promotions) {
        int target = popLSB(promotions);
        pseudoMoves.emplace_back(Move(target + singlePushOffset, target, PROMOTEBISHOP));
        pseudoMoves.emplace_back(Move(target + singlePushOffset, target, PROMOTEKNIGHT));
        pseudoMoves.emplace_back(Move(target + singlePushOffset, target, PROMOTEROOK));
        pseudoMoves.emplace_back(Move(target + singlePushOffset, target, PROMOTEQUEEN));
    }

    int doublePushOffset = b.turn ? -16 : 16;
    while (doublePushes) {
        int target = popLSB(doublePushes);
        pseudoMoves.emplace_back(Move(target + doublePushOffset, target, PAWNTWOFORWARD));
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
        pseudoMoves.emplace_back(Move(target + leftDiagonalAttackOffset, target, NONE));
    }

    while (leftDiagonalAttackPromotions) {
        int target = popLSB(leftDiagonalAttackPromotions);
        pseudoMoves.emplace_back(Move(target + leftDiagonalAttackOffset, target, PROMOTEBISHOP));
        pseudoMoves.emplace_back(Move(target + leftDiagonalAttackOffset, target, PROMOTEKNIGHT));
        pseudoMoves.emplace_back(Move(target + leftDiagonalAttackOffset, target, PROMOTEROOK));
        pseudoMoves.emplace_back(Move(target + leftDiagonalAttackOffset, target, PROMOTEQUEEN));
    }

    int rightDiagonalAttackOffset = b.turn ? -7 : 9;
    while (rightDiagonalAttacks) {
        int target = popLSB(rightDiagonalAttacks);
        pseudoMoves.emplace_back(Move(target + rightDiagonalAttackOffset, target, NONE));
    }

    while (rightDiagonalAttackPromotions) {
        int target = popLSB(rightDiagonalAttackPromotions);
        pseudoMoves.emplace_back(Move(target + rightDiagonalAttackOffset, target, PROMOTEBISHOP));
        pseudoMoves.emplace_back(Move(target + rightDiagonalAttackOffset, target, PROMOTEKNIGHT));
        pseudoMoves.emplace_back(Move(target + rightDiagonalAttackOffset, target, PROMOTEROOK));
        pseudoMoves.emplace_back(Move(target + rightDiagonalAttackOffset, target, PROMOTEQUEEN));
    }
}

void MoveGen::generateKnightMoves(const Board& b) {
    uint64_t knightBitboard = b.turn ? b.pieceBitboards[WKNIGHT] : b.pieceBitboards[BKNIGHT];
    uint64_t enemyOrEmpty = b.turn ? ~b.getWhitePositions() : ~b.getBlackPositions();
    uint64_t enemyKingBitboard = b.turn ? b.pieceBitboards[BKING] : b.pieceBitboards[WKING];
    uint64_t forcingMask = onlyGenerateForcing ? b.blockers | knightAttackBitboards[popLSB(enemyKingBitboard)] : ~0ULL;

    while (knightBitboard) {
        int source = popLSB(knightBitboard);
        uint64_t movesBitboard = knightAttackBitboards[source] & enemyOrEmpty & forcingMask;

        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            pseudoMoves.emplace_back(Move(source, target, NONE));
        }
    }
}

void MoveGen::generateSlidingMoves(const Board& b) {
    uint64_t bishopBitboard = b.turn ? b.pieceBitboards[WBISHOP] : b.pieceBitboards[BBISHOP];
    uint64_t rookBitboard = b.turn ? b.pieceBitboards[WROOK] : b.pieceBitboards[BROOK];
    uint64_t queenBitboard = b.turn ? b.pieceBitboards[WQUEEN] : b.pieceBitboards[BQUEEN];
    uint64_t enemyKingBitboard = b.turn ? b.pieceBitboards[BKING] : b.pieceBitboards[WKING];
    int enemyKingSquare = popLSB(enemyKingBitboard);

    // Generate forcing masks
    uint64_t bishopForcingMask = ~0ULL;
    uint64_t rookForcingMask = ~0ULL;
    uint64_t queenForcingMask = ~0ULL;

    if (onlyGenerateForcing) {
        uint64_t kingOccupancyBishop = bishopAttackMagicMasks[enemyKingSquare] & b.blockers;
        int kbIndex = BISHOP_ATTACKS_PER_SQUARE * enemyKingSquare + ((bishopMagics[enemyKingSquare] * kingOccupancyBishop) >> 55);
        bishopForcingMask = b.blockers | bishopAttackBitboards[kbIndex];

        uint64_t kingOccupancyRook = rookAttackMagicMasks[enemyKingSquare] & b.blockers;
        int krIndex = ROOK_ATTACKS_PER_SQUARE * enemyKingSquare + ((rookMagics[enemyKingSquare] * kingOccupancyRook) >> 52);
        rookForcingMask = b.blockers | rookAttackBitboards[krIndex];

        queenForcingMask = bishopForcingMask | rookForcingMask;
    }

    // Bishop moves
    while (bishopBitboard) {
        int source = popLSB(bishopBitboard);

        uint64_t occupancy = bishopAttackMagicMasks[source] & b.blockers;
        int index = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * occupancy) >> 55);
        uint64_t movesBitboard = bishopAttackBitboards[index] & ~friendly & bishopForcingMask;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            pseudoMoves.emplace_back(Move(source, target, NONE));
        }
    }

    // Rook moves
    while (rookBitboard) {
        int source = popLSB(rookBitboard);

        uint64_t occupancy = rookAttackMagicMasks[source] & b.blockers;
        int index = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * occupancy) >> 52);
        uint64_t movesBitboard = rookAttackBitboards[index] & ~friendly & rookForcingMask;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            pseudoMoves.emplace_back(Move(source, target, NONE));
        }
    }

    // Queen moves
    while (queenBitboard) {
        int source = popLSB(queenBitboard);

        uint64_t bishopOccupancy = bishopAttackMagicMasks[source] & b.blockers;
        int bishopIndex = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * bishopOccupancy) >> 55);

        uint64_t rookOccupancy = rookAttackMagicMasks[source] & b.blockers;
        int rookIndex = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * rookOccupancy) >> 52);

        uint64_t movesBitboard = (bishopAttackBitboards[bishopIndex] | rookAttackBitboards[rookIndex]) & ~friendly & queenForcingMask;
        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            pseudoMoves.emplace_back(Move(source, target, NONE));
        }
    }
}

void MoveGen::generateKingMoves(const Board& b) {
    uint64_t kingBitboard = b.turn ? b.pieceBitboards[WKING] : b.pieceBitboards[BKING];
    uint64_t forcingMask = onlyGenerateForcing ? b.blockers : ~0ULL;

    while (kingBitboard) {
        int source = popLSB(kingBitboard);
        uint64_t movesBitboard = kingAttackBitboards[source] & enemyOrEmpty & forcingMask;

        while (movesBitboard) {
            int target = popLSB(movesBitboard);
            pseudoMoves.emplace_back(Move(source, target, NONE));
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
            pseudoMoves.emplace_back(Move(E1, G1, CASTLING));
        }

        if ((b.history.top().castlingRights & W_Q) &&
            !(blockers & W_Q_EMPTY_MASK) &&
            !(attackedByOpp & W_Q_SAFE_MASK)) {
            pseudoMoves.emplace_back(Move(E1, C1, CASTLING));
        }
    } else {
        // Black to move
        if ((b.history.top().castlingRights & B_K) &&
            !(blockers & B_K_EMPTY_MASK) &&
            !(attackedByOpp & B_K_SAFE_MASK)) {
            pseudoMoves.emplace_back(Move(E8, G8, CASTLING));
        }
        if ((b.history.top().castlingRights & B_Q) &&
            !(blockers & B_Q_EMPTY_MASK) &&
            !(attackedByOpp & B_Q_SAFE_MASK)) {
            pseudoMoves.emplace_back(Move(E8, C8, CASTLING));
        }
    }
}

void MoveGen::orderMoves(const Board& b, uint16_t bestMoveValue) {
    // Assign score to each move
    for (Move& move : pseudoMoves) {
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
    sort(pseudoMoves.begin(), pseudoMoves.end(), [](const Move &a, const Move &b) {
        return a.moveScore > b.moveScore; 
    });

    // b.displayBoard();
    // for (Move move : moves) {
    //     cout << move << " | " << move.moveScore << " | " << bestMoveValue << endl;
    // }
}

void MoveGen::clearMoves() {
    pseudoMoves.clear();
}

static inline string squareToNotation(int sq) {
    // Your indexing: a8=63 ... h1=0
    int fileIdx = 7 - (sq % 8); // 0..7 -> a..h
    int rankIdx = (sq / 8);     // 0..7 -> rank 1..8 minus 1
    char fileChar = static_cast<char>('a' + fileIdx);
    char rankChar = static_cast<char>('1' + rankIdx);
    return string() + fileChar + rankChar;
}

void MoveGen::getLegalMovesDTO(Board& b, vector<LegalMoveDTO>& out) {
    out.clear();
    generateLegalMoves(b);
    out.reserve(legalMoves.size());
    for (const Move& m : legalMoves) {
        LegalMoveDTO dto;
        dto.id = m.moveValue;
        dto.from = squareToNotation(m.getSource());
        dto.to = squareToNotation(m.getTarget());
        dto.flag = static_cast<int>(m.getFlag());
        // Ignore promotions for now; flags will reflect NONE/CASTLING/ENPASSANT/PAWNTWOFORWARD
        out.emplace_back(std::move(dto));
    }
}