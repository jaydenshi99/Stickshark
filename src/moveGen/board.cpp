#include "../../include/board.h"

using namespace std;

Board::Board() : pieces{0ULL} {
    turn = true;
}

uint64_t Board::getBoardPositions() const {
    return Board::getWhitePositions() | Board::getBlackPositions();
}

uint64_t Board::getWhitePositions() const {
    return pieces[WPAWN] | pieces[WBISHOP] | pieces[WKNIGHT] | pieces[WROOK] | pieces[WQUEEN] | pieces[WKING];
}

uint64_t Board::getBlackPositions() const {
    return pieces[BPAWN] | pieces[BBISHOP] | pieces[BKNIGHT] | pieces[BROOK] | pieces[BQUEEN] | pieces[BKING];
}

void Board::setStartingPosition() {
    // White pieces
    pieces[WPAWN]   = 0x000000000000FF00;
    pieces[WBISHOP] = 0x0000000000000024;
    pieces[WKNIGHT] = 0x0000000000000042;
    pieces[WROOK]   = 0x0000000000000081;
    pieces[WQUEEN]  = 0x0000000000000008;
    pieces[WKING]   = 0x0000000000000010;

    // Black pieces
    pieces[BPAWN]   = 0x00FF000000000000;
    pieces[BBISHOP] = 0x2400000000000000;
    pieces[BKNIGHT] = 0x4200000000000000;
    pieces[BROOK]   = 0x8100000000000000;
    pieces[BQUEEN]  = 0x0800000000000000;
    pieces[BKING]   = 0x1000000000000000;
}

void Board::swapTurn() {
    turn = !turn;
}


void Board::displayBoard() const {
    // Create an array to represent the board
    char board[64];
    for (int i = 0; i < 64; ++i) {
        board[i] = '.';
    }

    // Define characters for each piece
    const char pieceSymbols[12] = {
        'P', 'B', 'N', 'R', 'Q', 'K', // White pieces
        'p', 'b', 'n', 'r', 'q', 'k'  // Black pieces
    };

    // Map each piece's bitboard to the board array
    for (int piece = 0; piece < 12; ++piece) {
        uint64_t bitboard = pieces[piece];
        while (bitboard) {
            int square = __builtin_ctzll(bitboard);
            board[square] = pieceSymbols[piece];
            bitboard &= bitboard - 1;
        }
    }

    // Print the board in reverse rank order (rank 8 to rank 1)
    std::cout << "\n  +------------------------+" << std::endl;
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << " | ";
        for (int file = 0; file < 8; ++file) {
            std::cout << board[rank * 8 + file] << " ";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "  +------------------------+" << std::endl;
    std::cout << "    a b c d e f g h" << std::endl;
}