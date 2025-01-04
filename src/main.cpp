#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    Board board = Board();
    board.setFEN("rnbqkb1r/pp3ppp/4pn2/2pp4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 0 1");

    Engine engine = Engine(board);
    engine.findBestMove(5);
    
    return 0;
}