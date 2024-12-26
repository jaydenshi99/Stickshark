#include "gamestate.h"

using namespace std;

Gamestate::Gamestate(uint64_t pieceBitboards[12], int cptPiece) {
    uint64_t blockers = pieceBitboards[WPAWN] | pieceBitboards[WBISHOP] | pieceBitboards[WKNIGHT] | pieceBitboards[WROOK] | pieceBitboards[WQUEEN] | pieceBitboards[WKING] |
    pieceBitboards[BPAWN] | pieceBitboards[BBISHOP] | pieceBitboards[BKNIGHT] | pieceBitboards[BROOK] | pieceBitboards[BQUEEN] | pieceBitboards[BKING];

    whiteAttacks = 0;
    blackAttacks = 0;

    // Pawns
    whiteAttacks |= (pieceBitboards[WPAWN] << 7) & notFileBitboards[0];
    whiteAttacks |= (pieceBitboards[WPAWN] << 9) & notFileBitboards[7];
    blackAttacks |= (pieceBitboards[BPAWN] >> 9) & notFileBitboards[0];
    blackAttacks |= (pieceBitboards[BPAWN] >> 7) & notFileBitboards[7];

    // Knights
    uint64_t wKnightBB = pieceBitboards[WKNIGHT];
    while (wKnightBB) {
        whiteAttacks |= knightAttackBitboards[popLSB(wKnightBB)];
    }

    uint64_t bKnightBB = pieceBitboards[BKNIGHT];
    while (bKnightBB) {
        blackAttacks |= knightAttackBitboards[popLSB(bKnightBB)];
    }

    // Bishops
    uint64_t wBishopBB = pieceBitboards[WBISHOP];
    while (wBishopBB) {
        int source = popLSB(wBishopBB);

        uint64_t occupancy = bishopAttackMagicMasks[source] & blockers;
        int index = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * occupancy) >> 55);
        whiteAttacks |= bishopAttackBitboards[index];
    }

    uint64_t bBishopBB = pieceBitboards[BBISHOP];
    while (bBishopBB) {
        int source = popLSB(bBishopBB);

        uint64_t occupancy = bishopAttackMagicMasks[source] & blockers;
        int index = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * occupancy) >> 55);
        blackAttacks |= bishopAttackBitboards[index];
    }

    // Rooks
    uint64_t wRookBB = pieceBitboards[WROOK];
    while (wRookBB) {
        int source = popLSB(wRookBB);

        uint64_t occupancy = rookAttackMagicMasks[source] & blockers;
        int index = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * occupancy) >> 52);
        whiteAttacks |= rookAttackBitboards[index];
    }

    uint64_t bRookBB = pieceBitboards[BROOK];
    while (bRookBB) {
        int source = popLSB(bRookBB);

        uint64_t occupancy = rookAttackMagicMasks[source] & blockers;
        int index = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * occupancy) >> 52);
        blackAttacks |= rookAttackBitboards[index];
    }

    // Queens
    uint64_t wQueenBB = pieceBitboards[WQUEEN];
    while (wQueenBB) {
        int source = popLSB(wQueenBB);

        uint64_t bishopOccupancy = bishopAttackMagicMasks[source] & blockers;
        int bishopIndex = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * bishopOccupancy) >> 55);

        uint64_t rookOccupancy = rookAttackMagicMasks[source] & blockers;
        int rookIndex = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * rookOccupancy) >> 52);

        whiteAttacks |= bishopAttackBitboards[bishopIndex] | rookAttackBitboards[rookIndex];
    }

    uint64_t bQueenBB = pieceBitboards[BQUEEN];
    while (bQueenBB) {
        int source = popLSB(bQueenBB);

        uint64_t bishopOccupancy = bishopAttackMagicMasks[source] & blockers;
        int bishopIndex = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * bishopOccupancy) >> 55);

        uint64_t rookOccupancy = rookAttackMagicMasks[source] & blockers;
        int rookIndex = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * rookOccupancy) >> 52);

        blackAttacks |= bishopAttackBitboards[bishopIndex] | rookAttackBitboards[rookIndex];
    }

    // Kings
    uint64_t wKingBB = pieceBitboards[WKING];
    while (wKingBB) {
        whiteAttacks |= kingAttackBitboards[popLSB(wKingBB)];
    }

    uint64_t bKingBB = pieceBitboards[BKING];
    while (bKingBB) {
        blackAttacks |= kingAttackBitboards[popLSB(bKingBB)];
    }

    capturedPiece = cptPiece;
}
