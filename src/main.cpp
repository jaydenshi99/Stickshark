#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    Board board = Board();
    board.setStartingPosition();

    Engine engine = Engine(board);
    engine.findBestMove(6);
    
    return 0;
}