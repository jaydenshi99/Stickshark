#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    perft(6, STARTING_FEN);
    
    return 0;
}