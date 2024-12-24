#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    for (int i = 0; i < 64; i++) {
        cout << "Bishop Magic Attacks Square " << i << endl;
        displayBitboard(bishopAttackMagicMasks[i]);
        cout << "Rook Magic Attacks Square " << i << endl;
        displayBitboard(rookAttackMagicMasks[i]);
    }

    // simulateRandomMoves();
    
    return 0;
}