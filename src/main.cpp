#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    vector<uint64_t> occupancies = generateAllOccupancies(bishopAttackMagicMasks[30]);

    displayBitboard(bishopAttackMagicMasks[30]);
    for (uint64_t occupancy : occupancies) {
        displayBitboard(occupancy);
    }

    // simulateRandomMoves();
    
    return 0;
}