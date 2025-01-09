#include "board.h"

using namespace std;

// Initialise empty board
Board::Board() : pieceBitboards{0ULL} {
    // Set turn
    turn = true;

    // Set squares
    for (int i = 0; i < 64; i++) {
        squares[i] = EMPTY;
    }

    // Set pieceSquareEval
    pieceSquareEval = 0;

    // Set gamestate
    Gamestate gState = Gamestate(EMPTY);
    history.push(gState);

    // Initialise setAttackFunctions
    setAttackMethods[0] = &Board::setPawnAttacks;
    setAttackMethods[2] = &Board::setKnightAttacks;
    setAttackMethods[5] = &Board::setKingAttacks;
}

void Board::setBlockers() {
    blockers = getWhitePositions() | getBlackPositions();
}

uint64_t Board::getWhitePositions() const {
    return pieceBitboards[WPAWN] | pieceBitboards[WBISHOP] | pieceBitboards[WKNIGHT] | pieceBitboards[WROOK] | pieceBitboards[WQUEEN] | pieceBitboards[WKING];
}

uint64_t Board::getBlackPositions() const {
    return pieceBitboards[BPAWN] | pieceBitboards[BBISHOP] | pieceBitboards[BKNIGHT] | pieceBitboards[BROOK] | pieceBitboards[BQUEEN] | pieceBitboards[BKING];
}

uint64_t Board::getWhiteAttacks() const {
    const Gamestate& gamestate = history.top();
    return gamestate.attackBitboards[WPAWN] | gamestate.attackBitboards[WBISHOP] | gamestate.attackBitboards[WKNIGHT] | gamestate.attackBitboards[WROOK] | gamestate.attackBitboards[WQUEEN] | gamestate.attackBitboards[WKING];
}

uint64_t Board::getBlackAttacks() const {
    const Gamestate& gamestate = history.top();
    return gamestate.attackBitboards[BPAWN] | gamestate.attackBitboards[BBISHOP] | gamestate.attackBitboards[BKNIGHT] | gamestate.attackBitboards[BROOK] | gamestate.attackBitboards[BQUEEN] | gamestate.attackBitboards[BKING];
}

bool Board::kingInCheck() const {
    return pieceBitboards[turn ? BKING : WKING] & (turn ? getWhiteAttacks() : getBlackAttacks());
}

void Board::setFEN(string FEN) {
    for (int i = 0; i < NUM_PIECES; i++) {
        pieceBitboards[i] = 0ULL;
    }

    for (int i = 0; i < NUM_SQUARES; i++) {
        squares[i] = EMPTY;
    }

    int index = 0;
    int square = 63;
    while (FEN[index] != ' ') {
        char c = FEN[index++];

        if (c == '/') continue;

        int pieceIndex = charToPieceIndex(c);
        if (pieceIndex != -1) {
            squares[square] = pieceIndex;
            pieceBitboards[pieceIndex] |= 1ULL << square;
            square -= 1;
        } else {
            square -= c - '0';
        }
    }

    index++;

    turn = FEN[index] == 'w';

    // Set blockers
    setBlockers();

    // Set new gamestate
    Gamestate gState = Gamestate(EMPTY);

    // Set attack bitboards
    for (int i = 0; i < 6; i++) {
        if (isNonSliding[i]) {
            (this->*setAttackMethods[i])(gState, true);
            (this->*setAttackMethods[i])(gState, false);
        }
    }
    setSliderAttacks(gState);

    history.push(gState);

    setZobristHash();
    setPieceSquareEvaluation();
}

void Board::setStartingPosition() {
    setFEN(STARTING_FEN);
}

void Board::swapTurn() {
    turn = !turn;
    zobristHash ^= zobristBitstrings[768];
}

void Board::displayBoard() const {
    // Define characters for each piece
    const char pieceSymbols[12] = {
        'P', 'B', 'N', 'R', 'Q', 'K', // White pieces
        'p', 'b', 'n', 'r', 'q', 'k'  // Black pieces
    };

    // Print the board (a1 is bottom-left, h8 is top-right)
    cout << "  +-----------------+" << endl;
    for (int rank = 7; rank >= 0; --rank) { // Start from rank 8 to rank 1
        cout << rank + 1 << " | ";
        for (int file = 7; file >= 0; --file) { // Iterate from a to h
            int pieceType = squares[rank * 8 + file];
            if (pieceType == EMPTY) {
                cout << ". ";
            } else {
                cout << pieceSymbols[pieceType] << " ";
            }

        }
        cout << "|" << endl;
    }
    cout << "  +-----------------+" << endl;
    cout << "    a b c d e f g h" << endl << endl; // Files a to h

    if (turn) {
        cout << "White to move...";
    } else {
        cout << "Black to move...";
    }

    cout << "\n\n";
}

void Board::makeMove(const Move& move) {
    uint64_t sourceSquare = move.getSource();
    uint64_t targetSquare = move.getTarget();
    int moveFlag = move.getFlag();

    uint64_t sourceSquareMask = 1ULL << sourceSquare;
    uint64_t targetSquareMask = 1ULL << targetSquare;

    int movedPiece = squares[sourceSquare];
    int capturedPiece = squares[targetSquare];

    // Set new gamestate
    Gamestate gState = Gamestate(capturedPiece);
    Gamestate oldGamestate = history.top();

    // Copy attacks of non sliding pieces
    gState.attackBitboards[WPAWN] = oldGamestate.attackBitboards[WPAWN];
    gState.attackBitboards[BPAWN] = oldGamestate.attackBitboards[BPAWN];
    gState.attackBitboards[WKNIGHT] = oldGamestate.attackBitboards[WKNIGHT];
    gState.attackBitboards[BKNIGHT] = oldGamestate.attackBitboards[BKNIGHT];
    gState.attackBitboards[WKING] = oldGamestate.attackBitboards[WKING];
    gState.attackBitboards[BKING] = oldGamestate.attackBitboards[BKING];


    // Empty moved piece
    squares[sourceSquare] = EMPTY;
    pieceBitboards[movedPiece] ^= sourceSquareMask;
    zobristHash ^= zobristBitstrings[movedPiece * NUM_SQUARES + sourceSquare];
    pieceSquareEval -= pieceSquareTables[movedPiece][sourceSquare];
    blockers ^= sourceSquareMask;

    // Remove captured piece
    if (capturedPiece != EMPTY) {
        pieceBitboards[capturedPiece] ^= targetSquareMask;
        zobristHash ^= zobristBitstrings[capturedPiece * NUM_SQUARES + targetSquare];
        pieceSquareEval -= pieceSquareTables[capturedPiece][targetSquare];
    } else {
        // If no captured piece, then add the new blocker on the empty square
        blockers ^= targetSquareMask;
    }

    // Flag specific cases
    if (moveFlag == NONE || moveFlag == PAWNTWOFORWARD) {
        // Add moved piece to new square
        squares[targetSquare] = movedPiece;
        pieceBitboards[movedPiece] ^= targetSquareMask;
        zobristHash ^= zobristBitstrings[movedPiece * NUM_SQUARES + targetSquare];
        pieceSquareEval += pieceSquareTables[movedPiece][targetSquare];
    } else if (isPromotion[moveFlag]) {
        // Add promoted piece to new square
        int promotedPiece = moveFlag - 2 + (turn ? 0 : 6);

        squares[targetSquare] = promotedPiece;
        pieceBitboards[promotedPiece] ^= targetSquareMask;
        zobristHash ^= zobristBitstrings[promotedPiece * NUM_SQUARES + targetSquare];
        pieceSquareEval += pieceSquareTables[promotedPiece][targetSquare];

        // Update piece attacks of promoted piece
        if (isNonSliding[promotedPiece]) updatePieceAttacks(gState, promotedPiece);
    }

    // Update attack of moved piece
    if (isNonSliding[movedPiece]) updatePieceAttacks(gState, movedPiece);
    if (isNonSliding[capturedPiece]) updatePieceAttacks(gState, capturedPiece);

    // Update attack of sliding pieces
    setSliderAttacks(gState);

    // Push Gamestate
    history.push(gState);

    // Toggle turn
    swapTurn();
}

void Board::unmakeMove(const Move& move) {
    uint64_t oldSquare = move.getSource();
    uint64_t currSquare = move.getTarget();
    int moveFlag = move.getFlag();

    uint64_t oldSquareMask = 1ULL << oldSquare;
    uint64_t currSquareMask = 1ULL << currSquare;

    // Get old gamestate
    Gamestate gState = history.top();
    history.pop();

    int movedPiece = isPromotion[moveFlag] ? (turn ? BPAWN : WPAWN) : squares[currSquare];
    int capturedPiece = gState.capturedPiece;

    // Bring back moved piece to old square
    squares[oldSquare] = movedPiece;
    pieceBitboards[movedPiece] ^= oldSquareMask;
    zobristHash ^= zobristBitstrings[movedPiece * NUM_SQUARES + oldSquare];
    pieceSquareEval += pieceSquareTables[movedPiece][oldSquare];

    // Bring back captured piece
    squares[currSquare] = capturedPiece;
    blockers ^= oldSquareMask;
    if (capturedPiece != EMPTY) {
        pieceBitboards[capturedPiece] |= currSquareMask; 
        zobristHash ^= zobristBitstrings[capturedPiece * NUM_SQUARES + currSquare];
        pieceSquareEval += pieceSquareTables[capturedPiece][currSquare];
    } else {
        // If no piece was captured then remove the blocker
        blockers ^= currSquareMask;
    }


    // Flag specific cases
    if (moveFlag == NONE || moveFlag == PAWNTWOFORWARD) {
        // Remove moved piece from square that it was moved to
        pieceBitboards[movedPiece] ^= currSquareMask;
        zobristHash ^= zobristBitstrings[movedPiece * NUM_SQUARES + currSquare];
        pieceSquareEval -= pieceSquareTables[movedPiece][currSquare];
    } else if (isPromotion[moveFlag]) {
        // Remove promoted piece from square that it was promoted
        int promotedPiece = moveFlag - 2 + (turn ? 6 : 0);

        pieceBitboards[promotedPiece] ^= currSquareMask;
        zobristHash ^= zobristBitstrings[promotedPiece * NUM_SQUARES + currSquare];
        pieceSquareEval -= pieceSquareTables[promotedPiece][currSquare];
    }

    // Toggle turn
    swapTurn();
}

void Board::updatePieceAttacks(Gamestate& gamestate, int piece) {
    bool white = piece < 6;
    (this->*setAttackMethods[piece % 6])(gamestate, white);
}

void Board::setPawnAttacks(Gamestate& gamestate, bool white) {
    if (white) {
        gamestate.attackBitboards[WPAWN] = (pieceBitboards[WPAWN] << 7) & notFileBitboards[0];
        gamestate.attackBitboards[WPAWN] |= (pieceBitboards[WPAWN] << 9) & notFileBitboards[7];
    } else {
        gamestate.attackBitboards[BPAWN] = (pieceBitboards[BPAWN] >> 9) & notFileBitboards[0];
        gamestate.attackBitboards[BPAWN] |= (pieceBitboards[BPAWN] >> 7) & notFileBitboards[7];
    }
}

void Board::setKnightAttacks(Gamestate& gamestate, bool white) {
    int pieceIndex = white ? WKNIGHT : BKNIGHT;
    gamestate.attackBitboards[pieceIndex] = 0ULL;

    uint64_t knightBB = pieceBitboards[pieceIndex];
    while (knightBB) {
        gamestate.attackBitboards[pieceIndex] |= knightAttackBitboards[popLSB(knightBB)];
    }
}

void Board::setKingAttacks(Gamestate& gamestate, bool white) {
    int pieceIndex = white ? WKING : BKING;
    gamestate.attackBitboards[pieceIndex] = 0ULL;

    uint64_t kingBB = pieceBitboards[pieceIndex];
    while (kingBB) {
        gamestate.attackBitboards[pieceIndex] |= kingAttackBitboards[popLSB(kingBB)];
    }
}

void Board::setSliderAttacks(Gamestate& gamestate) {
    // Helper lambda to calculate attacks for sliding pieces
    auto calculateAttacks = [&](uint64_t pieceBB, uint64_t *magicMasks, uint64_t *magics,
                                uint64_t *attackBitboards, int attacksPerSquare, int shift) {
        uint64_t attacks = 0ULL;
        while (pieceBB) {
            int source = popLSB(pieceBB);

            uint64_t occupancy = magicMasks[source] & blockers;
            int index = attacksPerSquare * source + ((magics[source] * occupancy) >> shift);

            attacks |= attackBitboards[index];
        }
        return attacks;
    };

    // Bishops
    gamestate.attackBitboards[WBISHOP] = calculateAttacks(pieceBitboards[WBISHOP],
    bishopAttackMagicMasks, bishopMagics, bishopAttackBitboards, BISHOP_ATTACKS_PER_SQUARE, 55);

    gamestate.attackBitboards[BBISHOP] = calculateAttacks(pieceBitboards[BBISHOP],
    bishopAttackMagicMasks, bishopMagics, bishopAttackBitboards, BISHOP_ATTACKS_PER_SQUARE, 55);

    // Rooks
    gamestate.attackBitboards[WROOK] = calculateAttacks(pieceBitboards[WROOK],
    rookAttackMagicMasks, rookMagics, rookAttackBitboards, ROOK_ATTACKS_PER_SQUARE, 52);

    gamestate.attackBitboards[BROOK] = calculateAttacks(pieceBitboards[BROOK],
    rookAttackMagicMasks, rookMagics, rookAttackBitboards, ROOK_ATTACKS_PER_SQUARE, 52);

    // Queens
    auto calculateQueenAttacks = [&](uint64_t queenBB, uint64_t *bishopMasks, uint64_t *bishopMagics,
                                     uint64_t *rookMasks, uint64_t *rookMagics) {
        uint64_t attacks = 0ULL;
        while (queenBB) {
            int source = popLSB(queenBB);

            uint64_t bishopOccupancy = bishopMasks[source] & blockers;
            int bishopIndex = BISHOP_ATTACKS_PER_SQUARE * source + ((bishopMagics[source] * bishopOccupancy) >> 55);

            uint64_t rookOccupancy = rookMasks[source] & blockers;
            int rookIndex = ROOK_ATTACKS_PER_SQUARE * source + ((rookMagics[source] * rookOccupancy) >> 52);

            attacks |= bishopAttackBitboards[bishopIndex] | rookAttackBitboards[rookIndex];
        }
        return attacks;
    };

    gamestate.attackBitboards[WQUEEN] = calculateQueenAttacks(pieceBitboards[WQUEEN],
    bishopAttackMagicMasks, bishopMagics, rookAttackMagicMasks, rookMagics);

    gamestate.attackBitboards[BQUEEN] = calculateQueenAttacks(pieceBitboards[BQUEEN],
    bishopAttackMagicMasks, bishopMagics, rookAttackMagicMasks, rookMagics);
}

void Board::setZobristHash() {
    // 0 - 767: piece positions
    for (int square = 0; square < NUM_SQUARES; square++) {
        if (squares[square] == EMPTY) {
            continue;
        }

        zobristHash ^= zobristBitstrings[squares[square] * NUM_SQUARES + square];
    }

    // 768: turn
    if (turn) {
        zobristHash ^= zobristBitstrings[768];
    }
}

void Board::setPieceSquareEvaluation() {
    pieceSquareEval = 0;
    for (int i = 0; i < 64; i++) {
        if (squares[i] != EMPTY) {
            pieceSquareEval += pieceSquareTables[squares[i]][i];
        }
    }
}

int charToPieceIndex (char c) {
    switch (c) {
        case 'P': return WPAWN;
        case 'B': return WBISHOP;
        case 'N': return WKNIGHT;
        case 'R': return WROOK;
        case 'Q': return WQUEEN;
        case 'K': return WKING;
        case 'p': return BPAWN;
        case 'b': return BBISHOP;
        case 'n': return BKNIGHT;
        case 'r': return BROOK;
        case 'q': return BQUEEN;
        case 'k': return BKING;
        default: return -1; // Invalid character
    }
}