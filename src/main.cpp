#include "main.h"
#include "utility.h"

using namespace std;

int main () {
    Board board;

    board.setStartingPosition();
    board.displayBoard();
    displayBitboard(board.pieceBitboards[BPAWN]);
    displayBitboard(board.pieceBitboards[BBISHOP]);
    displayBitboard(board.pieceBitboards[BKNIGHT]);
    displayBitboard(board.pieceBitboards[BROOK]);
    displayBitboard(board.pieceBitboards[BQUEEN]);
    displayBitboard(board.pieceBitboards[BKING]);

    Move m = Move(50, 30, 0);

    board.makeMove(m);
    board.displayBoard();
    displayBitboard(board.pieceBitboards[BPAWN]);
    displayBitboard(board.pieceBitboards[BBISHOP]);
    displayBitboard(board.pieceBitboards[BKNIGHT]);
    displayBitboard(board.pieceBitboards[BROOK]);
    displayBitboard(board.pieceBitboards[BQUEEN]);
    displayBitboard(board.pieceBitboards[BKING]);

    board.unmakeMove(m);
    board.displayBoard();
    displayBitboard(board.pieceBitboards[BPAWN]);
    displayBitboard(board.pieceBitboards[BBISHOP]);
    displayBitboard(board.pieceBitboards[BKNIGHT]);
    displayBitboard(board.pieceBitboards[BROOK]);
    displayBitboard(board.pieceBitboards[BQUEEN]);
    displayBitboard(board.pieceBitboards[BKING]);
    
    return 0;
}