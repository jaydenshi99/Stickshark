#include "../include/main.h"

using namespace std;

int main () {
    Board board;
    board.displayBoard();
    board.setStartingPosition();
    board.displayBoard();

    return 0;
}