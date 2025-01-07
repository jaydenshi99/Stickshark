#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    // playAI();

    perft(6, STARTING_FEN);

    return 0;
}