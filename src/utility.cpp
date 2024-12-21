#include "../include/utility.h"

using namespace std;

void displayBitboard(uint64_t bb) {
    cout << endl;
    for (int i = 7; i >= 0; --i) {
        for (int j = 7; j >= 0; --j) {
            int bit = (bb >> (i * 8 + j)) & 1ULL;

            cout << bit << " ";
        }
        cout << endl;
    }
}