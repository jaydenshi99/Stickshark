#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    Board board = Board();
    board.setFEN(STARTING_FEN);

    Engine engine = Engine(board);
    engine.findBestMove(5);

    perft(5, STARTING_FEN);
    
    return 0;
}