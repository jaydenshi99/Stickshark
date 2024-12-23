#include "bitTables.h"

void computeAllTables() {
    computeNotBitboards();
}

void computeNotBitboards() {
    for (int i = 0; i < 8; i++) {
        notFileBitboards[i] = ~fileBitboards[i];
        notRankBitboards[i] = ~rankBitboards[i];
    }
}