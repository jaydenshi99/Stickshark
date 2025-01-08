#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    // engineVSEngine(STARTING_FEN);

    Board board = Board();
    board.setStartingPosition();
    
    Engine engine = Engine(board);
    engine.findBestMove(34);

    return 0;
}