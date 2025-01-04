#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    Board board = Board();
    board.setFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");

    cout << evaluateBoard(board) << endl;

    Engine engine = Engine(board);
    engine.findBestMove(5);
    
    return 0;
}