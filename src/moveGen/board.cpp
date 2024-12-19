#include "../../include/board.h"

using namespace std;

Board::Board() : pieceBitboards{0ULL}, squares{0} {
    turn = true;
}

uint64_t Board::getBoardPositions() const {
    return Board::getWhitePositions() | Board::getBlackPositions();
}

uint64_t Board::getWhitePositions() const {
    return pieceBitboards[WPAWN] | pieceBitboards[WBISHOP] | pieceBitboards[WKNIGHT] | pieceBitboards[WROOK] | pieceBitboards[WQUEEN] | pieceBitboards[WKING];
}

uint64_t Board::getBlackPositions() const {
    return pieceBitboards[BPAWN] | pieceBitboards[BBISHOP] | pieceBitboards[BKNIGHT] | pieceBitboards[BROOK] | pieceBitboards[BQUEEN] | pieceBitboards[BKING];
}

void Board::setStartingPosition() {
    // White pieces
    pieceBitboards[WPAWN]   = 0x000000000000FF00;
    pieceBitboards[WBISHOP] = 0x0000000000000024;
    pieceBitboards[WKNIGHT] = 0x0000000000000042;
    pieceBitboards[WROOK]   = 0x0000000000000081;
    pieceBitboards[WQUEEN]  = 0x0000000000000008;
    pieceBitboards[WKING]   = 0x0000000000000010;

    // Black pieces
    pieceBitboards[BPAWN]   = 0x00FF000000000000;
    pieceBitboards[BBISHOP] = 0x2400000000000000;
    pieceBitboards[BKNIGHT] = 0x4200000000000000;
    pieceBitboards[BROOK]   = 0x8100000000000000;
    pieceBitboards[BQUEEN]  = 0x0800000000000000;
    pieceBitboards[BKING]   = 0x1000000000000000;

    int startingSquares[64] = {
        9, 8, 7, 10, 11, 7, 8, 9,
        6, 6, 6, 6, 6, 6, 6, 6,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        0, 0, 0, 0, 0, 0, 0, 0,
        3, 2, 1, 4, 5, 1, 2, 3
    };

    for (int i = 0; i < 64; ++i) {
        squares[i] = startingSquares[i];
    }
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
        uint64_t bitboard = pieceBitboards[piece];
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

void Board::makeMove(Move move) {
    uint64_t startSquare = move.getStart();
    uint64_t targetSquare = move.getTarget();

    uint64_t startSquareMask = 1ULL << startSquare;
    uint64_t targetSquareMask = 1ULL << targetSquare;

    int movedPiece = squares[startSquare];
    attackedPiece = squares[targetSquare];

    // Update 

    // Update Bitboards
    pieceBitboards[movedPiece] ^= startSquareMask | targetSquareMask;
    if (attackedPiece != EMPTY) {
        pieceBitboards[attackedPiece] &= ~targetSquareMask; 
    }

    // Update Squares
    squares[startSquare] = EMPTY;
    squares[targetSquare] = movedPiece;
}

void Board::unmakeMove(Move move) {
    uint64_t oldSquare = move.getStart();
    uint64_t currSquare = move.getTarget();

    uint64_t oldSquareMask = 1ULL << oldSquare;
    uint64_t currSquareMask = 1ULL << currSquare;

    int movedPiece = squares[currSquare];

    

}