#include "board.h"

using namespace std;

// Initialise empty board
Board::Board() : pieceBitboards{0ULL}, squares{0} {
    // Set turn
    turn = true;

    // Set gamestate
    Gamestate gState = Gamestate(EMPTY);
    history.push(gState);

    // Initialise setAttackFunctions
    setAttackFunctions[0] = &Board::setPawnAttacks;
    setAttackFunctions[1] = &Board::setKnightAttacks;
    setAttackFunctions[2] = &Board::setBishopAttacks;
    setAttackFunctions[3] = &Board::setRookAttacks;
    setAttackFunctions[4] = &Board::setQueenAttacks;
    setAttackFunctions[5] = &Board::setKingAttacks;

    // Calculate precomputed bitboards

    // Initialise attack bitboards
    for (int i = 0; i < NUM_PIECES; i++) {
        (this->*setAttackFunctions[i])(true);
        (this->*setAttackFunctions[i])(false);
    }
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

uint64_t Board::getWhiteAttacks() const {
    return attackBitboards[WPAWN] | attackBitboards[WBISHOP] | attackBitboards[WKNIGHT] | attackBitboards[WROOK] | attackBitboards[WQUEEN] | attackBitboards[WKING];
}

uint64_t Board::getBlackAttacks() const {
    return attackBitboards[BPAWN] | attackBitboards[BBISHOP] | attackBitboards[BKNIGHT] | attackBitboards[BROOK] | attackBitboards[BQUEEN] | attackBitboards[BKING];
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

    history.push(Gamestate(EMPTY));

    // Set attack bitboards
    for (int i = 0; i < NUM_PIECES; i++) {
        (this->*setAttackFunctions[i])(true);
        (this->*setAttackFunctions[i])(false);
    }
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
    uint64_t sourceSquare = move.getSource();
    uint64_t targetSquare = move.getTarget();

    uint64_t sourceSquareMask = 1ULL << sourceSquare;
    uint64_t targetSquareMask = 1ULL << targetSquare;

    int movedPiece = squares[sourceSquare];
    int capturedPiece = squares[targetSquare];

    // Update Gamestate
    Gamestate gState = Gamestate(capturedPiece);

    // Update Bitboards
    pieceBitboards[movedPiece] ^= sourceSquareMask | targetSquareMask;
    if (capturedPiece != EMPTY) {
        pieceBitboards[capturedPiece] ^= targetSquareMask;
        updatePieceAttacks(capturedPiece);
    }

    // Update Squares
    squares[sourceSquare] = EMPTY;
    squares[targetSquare] = movedPiece;

    // Toggle turn
    swapTurn();

    // Push Gamestate
    history.push(gState);

    // Update attack of moved piece
    updatePieceAttacks(movedPiece);
}

void Board::unmakeMove(Move move) {
    uint64_t oldSquare = move.getSource();
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
        updatePieceAttacks(capturedPiece);
    }

    // Toggle turn
    swapTurn();

    // Update Squares
    squares[currSquare] = capturedPiece;
    squares[oldSquare] = movedPiece;  

    // Update attack of moved piece
    updatePieceAttacks(movedPiece);
}

void Board::updatePieceAttacks(int piece) {
    bool white = piece < 6;
    (this->*setAttackFunctions[piece % 6])(white);
}

void Board::setPawnAttacks(bool white) {
    if (white) {
        attackBitboards[WPAWN] |= (pieceBitboards[WPAWN] << 7) & notFileBitboards[0];
        attackBitboards[WPAWN] |= (pieceBitboards[WPAWN] << 9) & notFileBitboards[7];
    } else {
        attackBitboards[BPAWN] |= (pieceBitboards[BPAWN] >> 9) & notFileBitboards[0];
        attackBitboards[BPAWN] |= (pieceBitboards[BPAWN] >> 7) & notFileBitboards[7];
    }
}

void Board::setKnightAttacks(bool white) {
    int pieceIndex = white ? WKNIGHT : BKNIGHT;

    uint64_t knightBB = pieceBitboards[pieceIndex];
    while (knightBB) {
        attackBitboards[pieceIndex] |= knightAttackBitboards[popLSB(knightBB)];
    }
}

void Board::setBishopAttacks(bool white) {
    int pieceIndex = white ? WBISHOP : BBISHOP;
    uint64_t blockers = getBlockers();

    uint64_t bishopBB = pieceBitboards[pieceIndex];
    while (bishopBB) {
        int source = popLSB(bishopBB);

        uint64_t occupancy = bishopAttackMagicMasks[source] & blockers;
        int index = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * occupancy) >> 55);
        attackBitboards[pieceIndex] |= bishopAttackBitboards[index];
    }
}

void Board::setRookAttacks(bool white) {
    int pieceIndex = white ? WROOK : BROOK;
    uint64_t blockers = getBlockers();

    uint64_t rookBB = pieceBitboards[pieceIndex];
    while (rookBB) {
        int source = popLSB(rookBB);

        uint64_t occupancy = rookAttackMagicMasks[source] & blockers;
        int index = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * occupancy) >> 52);
        attackBitboards[pieceIndex] |= rookAttackBitboards[index];
    }
}

void Board::setQueenAttacks(bool white) {
    int pieceIndex = white ? WQUEEN : BQUEEN;
    uint64_t blockers = getBlockers();

    uint64_t queenBB = pieceBitboards[pieceIndex];
    while (queenBB) {
        int source = popLSB(queenBB);

        uint64_t bishopOccupancy = bishopAttackMagicMasks[source] & blockers;
        int bishopIndex = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * bishopOccupancy) >> 55);

        uint64_t rookOccupancy = rookAttackMagicMasks[source] & blockers;
        int rookIndex = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * rookOccupancy) >> 52);

        attackBitboards[pieceIndex] |= bishopAttackBitboards[bishopIndex] | rookAttackBitboards[rookIndex];
    }
}

void Board::setKingAttacks(bool white) {
    int pieceIndex = white ? WKING : BKING;

    uint64_t kingBB = pieceBitboards[pieceIndex];
    while (kingBB) {
        attackBitboards[pieceIndex] |= kingAttackBitboards[popLSB(kingBB)];
    }
}