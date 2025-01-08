#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    // Board board = Board();
    // board.setFEN("8/3P4/8/8/8/8/8/8 w HAha - 0 1");
    // board.displayBoard();

    displayPossibleMoves("4p3/3P4/8/8/8/8/8/8 w HAha - 0 1");

    // Engine engine = Engine(board);
    // engine.findBestMove(1000);

    


    // perft(6, STARTING_FEN);

    return 0;
}