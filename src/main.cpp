#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    Board board = Board();
    board.setStartingPosition();

    playAI();

    return 0;
}