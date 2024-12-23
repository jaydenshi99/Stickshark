#include "board.h"

using namespace std;

Board::Board() : pieceBitboards{0ULL}, squares{0} {
    turn = true;
}

uint64_t Board::getBlockers() const {
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
    pieceBitboards[WQUEEN]  = 0x0000000000000010;
    pieceBitboards[WKING]   = 0x000000000000008;

    // Black pieces
    pieceBitboards[BPAWN]   = 0x00FF000000000000;
    pieceBitboards[BBISHOP] = 0x2400000000000000;
    pieceBitboards[BKNIGHT] = 0x4200000000000000;
    pieceBitboards[BROOK]   = 0x8100000000000000;
    pieceBitboards[BQUEEN]  = 0x1000000000000000;
    pieceBitboards[BKING]   = 0x0800000000000000;

    int startingSquares[64] = {
        3, 2, 1, 5, 4, 1, 2, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        6, 6, 6, 6, 6, 6, 6, 6,
        9, 8, 7, 11, 10, 7, 8, 9
    };

    for (int i = 0; i < 64; ++i) {
        squares[i] = startingSquares[i];
    }

    turn = true;
}

void Board::swapTurn() {
    turn = !turn;
}

void Board::displayBoard() const {
    // Define characters for each piece
    const char pieceSymbols[12] = {
        'P', 'B', 'N', 'R', 'Q', 'K', // White pieces
        'p', 'b', 'n', 'r', 'q', 'k'  // Black pieces
    };

    // Print the board (a1 is bottom-left, h8 is top-right)
    std::cout << "  +------------------------+" << std::endl;
    for (int rank = 7; rank >= 0; --rank) { // Start from rank 8 to rank 1
        std::cout << rank + 1 << " | ";
        for (int file = 7; file >= 0; --file) { // Iterate from a to h
            int pieceType = squares[rank * 8 + file];
            if (pieceType == EMPTY) {
                std::cout << ". ";
            } else {
                std::cout << pieceSymbols[pieceType] << " ";
            }

        }
        std::cout << "|" << std::endl;
    }
    std::cout << "  +------------------------+" << std::endl;
    std::cout << "    a b c d e f g h" << std::endl; // Files a to h
    cout << endl;
}

void Board::makeMove(Move move) {
    uint64_t startSquare = move.getStart();
    uint64_t targetSquare = move.getTarget();

    uint64_t startSquareMask = 1ULL << startSquare;
    uint64_t targetSquareMask = 1ULL << targetSquare;

    int movedPiece = squares[startSquare];
    int capturedPiece = squares[targetSquare];

    // Update Gamestate
    Gamestate gState = Gamestate(capturedPiece);

    // Update Bitboards
    pieceBitboards[movedPiece] ^= startSquareMask | targetSquareMask;
    if (capturedPiece != EMPTY) {
        pieceBitboards[capturedPiece] ^= targetSquareMask;
    }

    // Update Squares
    squares[startSquare] = EMPTY;
    squares[targetSquare] = movedPiece;

    // Toggle turn
    swapTurn();

    // Push Gamestate
    history.push(gState);
}

void Board::unmakeMove(Move move) {
    uint64_t oldSquare = move.getStart();
    uint64_t currSquare = move.getTarget();

    uint64_t oldSquareMask = 1ULL << oldSquare;
    uint64_t currSquareMask = 1ULL << currSquare;

    // Get old gamestate
    Gamestate gState = history.top();
    history.pop();

    int movedPiece = squares[currSquare];
    int capturedPiece = gState.capturedPiece;

    // Update Bitboards
    pieceBitboards[movedPiece] ^= oldSquareMask | currSquareMask;
    if (capturedPiece != EMPTY) {
        pieceBitboards[capturedPiece] |= currSquareMask; 
    }

    // Toggle turn
    swapTurn();

    // Update Squares
    squares[currSquare] = capturedPiece;
    squares[oldSquare] = movedPiece;
}