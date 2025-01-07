#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    Board board = Board();
    board.setFEN(STARTING_FEN);
    board.displayBoard();

    Engine engine = Engine(board);
    engine.findBestMove(700);


    // perft(6, STARTING_FEN);

    return 0;
}