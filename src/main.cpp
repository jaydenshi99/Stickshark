#include "../include/main.h"

using namespace std;

int main () {
    Board board;
    board.setStartingPosition();
    board.displayBoard();

    Move m = Move(0, 30, 0);
    board.makeMove(m);
    board.displayBoard();
    board.unmakeMove(m);
    board.displayBoard();
    
    return 0;
}