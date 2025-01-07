#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    Board board = Board();
    board.setFEN(STARTING_FEN);

    board.displayBoard();

    Engine engine = Engine(board);
    engine.findBestMove(10000);

    board.makeMove(engine.bestMove);
    board.displayBoard();

    // perft(6, STARTING_FEN);

    return 0;
}