#include "bitTables.h"

const uint64_t fileBitboards[NUMFILES] = {
    0x0101010101010101,
    0x0202020202020202,
    0x0404040404040404,
    0x0808080808080808,
    0x1010101010101010,
    0x2020202020202020,
    0x4040404040404040,
    0x8080808080808080
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


void computeAllTables() {
    computeNotBitboards();
}

void computeNotBitboards() {
    for (int i = 0; i < 8; i++) {
        notFileBitboards[i] = ~fileBitboards[i];
        notRankBitboards[i] = ~rankBitboards[i];
    }
}