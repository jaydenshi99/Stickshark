#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    int square = 27;
    vector<uint64_t> occupancies = generateAllOccupancies(rookAttackMagicMasks[square]);
    displayBitboard(rookAttackMagicMasks[square]);
    for (uint64_t occupancy : occupancies) {
        cout << "Blockers: " << endl;
        displayBitboard(occupancy);

        cout << "Attacks: " << endl;
        int index = ROOK_ATTACKS_PER_SQUARE * square + ((rookMagics[square] * occupancy) >> 52);
        displayBitboard(rookAttackBitboards[index]);
    }
    
    return 0;
}