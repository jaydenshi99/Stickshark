#include "bitTables.h"

const uint64_t fileBitboards[NUMFILES] = {
    0x8080808080808080,
    0x4040404040404040,
    0x2020202020202020,
    0x1010101010101010,
    0x0808080808080808,
    0x0404040404040404,
    0x0202020202020202,
    0x0101010101010101
};

const uint64_t rankBitboards[NUMRANKS] = {
    0x00000000000000FF,
    0x000000000000FF00,
    0x0000000000FF0000,
    0x00000000FF000000,
    0x000000FF00000000,
    0x0000FF0000000000,
    0x00FF000000000000,
    0xFF00000000000000
};

uint64_t notFileBitboards[NUMFILES];
uint64_t notRankBitboards[NUMRANKS];

uint64_t knightAttackBitboards[NUMSQUARES];


void computeAllTables() {
    computeNotBitboards();
    computeKnightAttacks();
}

void computeNotBitboards() {
    for (int i = 0; i < NUMFILES; i++) {
        notFileBitboards[i] = ~fileBitboards[i];
        notRankBitboards[i] = ~rankBitboards[i];
    }
}

void computeKnightAttacks() {
    for (int i = 0; i < NUMSQUARES; i++) {
        uint64_t knightPos = 1ULL << i;
        knightAttackBitboards[i] = (knightPos << 15) & notFileBitboards[0] & notRankBitboards[0] & notRankBitboards[1];
        knightAttackBitboards[i] |= (knightPos << 6) & notFileBitboards[0] & notFileBitboards[1] & notRankBitboards[0];
        knightAttackBitboards[i] |= (knightPos >> 10) & notFileBitboards[0] & notFileBitboards[1] & notRankBitboards[7];
        knightAttackBitboards[i] |= (knightPos >> 17) & notFileBitboards[0] & notRankBitboards[6] & notRankBitboards[7];
        knightAttackBitboards[i] |= (knightPos >> 15) & notFileBitboards[7] & notRankBitboards[6] & notRankBitboards[7];
        knightAttackBitboards[i] |= (knightPos >> 6) & notFileBitboards[6] & notFileBitboards[7] & notRankBitboards[7];
        knightAttackBitboards[i] |= (knightPos << 10) & notFileBitboards[6] & notFileBitboards[7] & notRankBitboards[0];
        knightAttackBitboards[i] |= (knightPos << 17) & notFileBitboards[7] & notRankBitboards[0] & notRankBitboards[1];
    }
}