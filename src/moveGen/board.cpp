#include "../../include/board.h"

using namespace std;

uint64_t Board::getBoardPositions() const {
    return Board::getWhitePositions() | Board::getBlackPositions();
}

uint64_t Board::getWhitePositions() const {
    return pieces[WPAWN] | pieces[WBISHOP] | pieces[WKNIGHT] | pieces[WROOK] | pieces[WQUEEN] | pieces[WKING];
}

uint64_t Board::getBlackPositions() const {
    return pieces[BPAWN] | pieces[BBISHOP] | pieces[BKNIGHT] | pieces[BROOK] | pieces[BQUEEN] | pieces[BKING];
}

void Board::swapTurn() {
    turn = !turn;
}