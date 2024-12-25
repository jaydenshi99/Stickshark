#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    vector<uint64_t> occupancies = generateAllOccupancies(rookAttackMagicMasks[30]);

    displayBitboard(rookAttackMagicMasks[30]);
    for (uint64_t occupancy : occupancies) {
        displayBitboard(occupancy);
    }

    // simulateRandomMoves();
    
    return 0;
}