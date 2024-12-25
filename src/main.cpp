#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    for (int i = 0; i < 64; i++) {
        cout << bitset<64>(rookMagics[i]) << endl;
    }
    
    return 0;
}