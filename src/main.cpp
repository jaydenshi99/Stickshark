#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    for (int i = 0; i < 64; i++) {
        cout << i << ". " << endl;
        displayBitboard(kingAttackBitboards[i]);
    }
    
    return 0;
}