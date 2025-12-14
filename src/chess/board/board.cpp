#include "board.h"

using namespace std;

// Initialise empty board
Board::Board() : pieceBitboards{0ULL} {
    // Set turn
    turn = true;

    // Set ply
    ply = 0;

    // Set squares
    for (int i = 0; i < 64; i++) {
        squares[i] = EMPTY;
    }

    // Set pieceSquareEval
    pieceSquareEvalMG = 0;
    pieceSquareEvalEG = 0;

    // Set gamestate
    Gamestate gState = Gamestate(EMPTY);
    history.push(gState);

    // Initialise setAttackFunctions
    setAttackMethods[0] = &Board::setPawnAttacks;
    setAttackMethods[2] = &Board::setKnightAttacks;
    setAttackMethods[5] = &Board::setKingAttacks;

    // Initialise zobrist history
    for (int i = 0; i < 4096; i++) {
        zobristHistory[i] = 0ULL;
        lastIrreversiblePly[i] = 0;
    }
}

void Board::setBlockers() {
    blockers = getWhitePositions() | getBlackPositions();
}

// moved inline to board.h for performance

void Board::setFEN(string FEN, bool clearRepetitionHistory) {
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
    while (!history.empty()) history.pop();
    Gamestate gState = Gamestate(EMPTY);

    // Set attack bitboards
    for (int i = 0; i < 6; i++) {
        if (isNonSliding[i]) {
            (this->*setAttackMethods[i])(gState, true);
            (this->*setAttackMethods[i])(gState, false);
        }
    }
    setSliderAttacks(gState);

    index += 2;

    // Set castling rights
    uint8_t castlingRights = 0;
    while (FEN[index] != ' ') {
        char c = FEN[index++];
        if (c == 'K') castlingRights |= 1;
        if (c == 'Q') castlingRights |= 2;
        if (c == 'k') castlingRights |= 4;
        if (c == 'q') castlingRights |= 8;
    }

    gState.castlingRights = castlingRights;

    index++; // skip space after castling

    // Set en passant
    if (FEN[index] == '-') {
        gState.enpassantColumn = -1;
        index += 2; // skip '-' and space
    } else {
        // Parse square (e.g., "e3" -> file 4, column 4)
        char file = FEN[index++];
        gState.enpassantColumn = 7 - (file - 'a'); // 'a' = 0, 'b' = 1, ..., 'h' = 7
        index++; // skip space
    }

    history.push(gState);

    setZobristHash();
    setPieceSquareEvaluation();

    if (clearRepetitionHistory) {
        for (int i = 0; i < 4096; i++) {
            zobristHistory[i] = 0ULL;
            lastIrreversiblePly[i] = 0;
        }
        ply = 0;
        zobristHistory[ply] = zobristHash;
        lastIrreversiblePly[ply] = ply;
    } else {
        zobristHistory[ply] = zobristHash;
        lastIrreversiblePly[ply] = lastIrreversiblePly[ply - 1];
    }
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
    int moveFlag = move.getFlag();
    uint64_t sourceSquare = move.getSource();
    uint64_t targetSquare = move.getTarget();
    uint64_t capturedSquare = (moveFlag == ENPASSANT) ? (turn ? targetSquare - 8 : targetSquare + 8) : targetSquare;


    uint64_t sourceSquareMask = 1ULL << sourceSquare;
    uint64_t targetSquareMask = 1ULL << targetSquare;
    uint64_t captureSquareMask = 1ULL << capturedSquare;

    int movedPiece = squares[sourceSquare];
    int capturedPiece = squares[capturedSquare];

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

    // Clear old enpassant column
    if (oldGamestate.enpassantColumn == -1) {
        zobristHash ^= zobristBitstrings[774]; // no enpassant column
    } else {
        zobristHash ^= zobristBitstrings[775 + oldGamestate.enpassantColumn];
    }

    // Empty moved piece
    squares[sourceSquare] = EMPTY;
    pieceBitboards[movedPiece] ^= sourceSquareMask;
    zobristHash ^= zobristBitstrings[movedPiece * NUM_SQUARES + sourceSquare];
    pieceSquareEvalMG -= pieceSquareTablesMG[movedPiece][sourceSquare];
    pieceSquareEvalEG -= pieceSquareTablesEG[movedPiece][sourceSquare];
    blockers ^= sourceSquareMask;

    // Remove captured piece
    if (capturedPiece != EMPTY) {
        pieceBitboards[capturedPiece] ^= captureSquareMask;
        zobristHash ^= zobristBitstrings[capturedPiece * NUM_SQUARES + capturedSquare];
        pieceSquareEvalMG -= pieceSquareTablesMG[capturedPiece][capturedSquare];
        pieceSquareEvalEG -= pieceSquareTablesEG[capturedPiece][capturedSquare];
        squares[capturedSquare] = EMPTY;
        blockers ^= captureSquareMask;
    }

    // Flag specific cases
    if (moveFlag == NONE || moveFlag == PAWNTWOFORWARD || moveFlag == ENPASSANT ||moveFlag == CASTLING) {
        // Add moved piece to new square
        squares[targetSquare] = movedPiece;
        pieceBitboards[movedPiece] ^= targetSquareMask;
        zobristHash ^= zobristBitstrings[movedPiece * NUM_SQUARES + targetSquare];
        pieceSquareEvalMG += pieceSquareTablesMG[movedPiece][targetSquare];
        pieceSquareEvalEG += pieceSquareTablesEG[movedPiece][targetSquare];
        blockers ^= targetSquareMask;

        if (moveFlag == PAWNTWOFORWARD) {
            gState.enpassantColumn = targetSquare % 8;
            zobristHash ^= zobristBitstrings[775 + gState.enpassantColumn]; // update zobrist hash for enpassant column
        } else {
            zobristHash ^= zobristBitstrings[774]; // no enpassant column
        }

    } else if (isPromotion[moveFlag]) {
        // Add promoted piece to new square
        int promotedPiece = moveFlag - 2 + (turn ? 0 : 6);

        squares[targetSquare] = promotedPiece;
        pieceBitboards[promotedPiece] ^= targetSquareMask;
        zobristHash ^= zobristBitstrings[promotedPiece * NUM_SQUARES + targetSquare];
        pieceSquareEvalMG += pieceSquareTablesMG[promotedPiece][targetSquare];
        pieceSquareEvalEG += pieceSquareTablesEG[promotedPiece][targetSquare];
        blockers ^= targetSquareMask;

        // Update piece attacks of promoted piece
        if (isNonSliding[promotedPiece]) updatePieceAttacks(gState, promotedPiece);
        
        zobristHash ^= zobristBitstrings[774]; // no enpassant column
    }
    
    // In castling moves, the rook is moved along with the king
    if (moveFlag == CASTLING) {
        // Determine rook movement based on target square
        int rookFrom = -1, rookTo = -1;
        int rookPiece = (movedPiece < 6) ? WROOK : BROOK;

        if (targetSquare == 1) { rookFrom = 0; rookTo = 2; }
        else if (targetSquare == 5) { rookFrom = 7; rookTo = 4; }
        else if (targetSquare == 57) { rookFrom = 56; rookTo = 58; }
        else if (targetSquare == 61) { rookFrom = 63; rookTo = 60; }

        // Move rook
        if (rookFrom != -1 && rookTo != -1) {
            uint64_t rFromMask = 1ULL << rookFrom;
            uint64_t rToMask   = 1ULL << rookTo;

            squares[rookFrom] = EMPTY;
            pieceBitboards[rookPiece] ^= rFromMask;
            zobristHash ^= zobristBitstrings[rookPiece * NUM_SQUARES + rookFrom];
            pieceSquareEvalMG -= pieceSquareTablesMG[rookPiece][rookFrom];
            pieceSquareEvalEG -= pieceSquareTablesEG[rookPiece][rookFrom];
            blockers ^= rFromMask;

            squares[rookTo] = rookPiece;
            pieceBitboards[rookPiece] ^= rToMask;
            zobristHash ^= zobristBitstrings[rookPiece * NUM_SQUARES + rookTo];
            pieceSquareEvalMG += pieceSquareTablesMG[rookPiece][rookTo];
            pieceSquareEvalEG += pieceSquareTablesEG[rookPiece][rookTo];
            blockers ^= rToMask;
        }
    }

    // Update castling rights (compute new mask, then apply Zobrist delta)
    uint8_t oldRights = oldGamestate.castlingRights;
    uint8_t rights    = oldRights;
    
    constexpr uint8_t W_K = 1u, W_Q = 2u, B_K = 4u, B_Q = 8u;
    
    const int s = (int)move.getSource();
    const int t = (int)move.getTarget();
    
    // 1) King moved => clear both
    if (movedPiece == WKING) rights &= (uint8_t)~(W_K | W_Q);
    if (movedPiece == BKING) rights &= (uint8_t)~(B_K | B_Q);
    
    // 2) Rook moved off its start square => clear that side’s right
    if (movedPiece == WROOK) {
        if (s == WKR_START_SQUARE) rights &= (uint8_t)~W_K;
        if (s == WQR_START_SQUARE) rights &= (uint8_t)~W_Q;
    }
    if (movedPiece == BROOK) {
        if (s == BKR_START_SQUARE) rights &= (uint8_t)~B_K;
        if (s == BQR_START_SQUARE) rights &= (uint8_t)~B_Q;
    }
    
    // 3) Rook captured on its start square => clear that side’s right
    if (capturedPiece == WROOK) {
        if (t == WKR_START_SQUARE) rights &= (uint8_t)~W_K;
        if (t == WQR_START_SQUARE) rights &= (uint8_t)~W_Q;
    }
    if (capturedPiece == BROOK) {
        if (t == BKR_START_SQUARE) rights &= (uint8_t)~B_K;
        if (t == BQR_START_SQUARE) rights &= (uint8_t)~B_Q;
    }

    uint8_t changed = oldRights ^ rights;
    if (changed) {
        static constexpr int zobristCastlingBase = 769;
        for (int i = 0; i < 4; ++i) {
            if (changed & (1u << i)) {
                zobristHash ^= zobristBitstrings[zobristCastlingBase + i];
            }
        }
        gState.castlingRights = rights;
    } else {
        gState.castlingRights = oldRights;
    }

    // Update attack of affected pieces if non-slidiing.
    if (isNonSliding[movedPiece]) updatePieceAttacks(gState, movedPiece);
    if (isNonSliding[capturedPiece]) updatePieceAttacks(gState, capturedPiece);

    // Update attack of sliding pieces
    setSliderAttacks(gState);

    // Push Gamestate
    history.push(gState);

    // Toggle turn
    swapTurn();

    // Update zobrist history
    zobristHistory[++ply] = zobristHash;

    // Update last irreversible ply
    if (moveFlag != NONE || capturedPiece != EMPTY || movedPiece == WPAWN || movedPiece == BPAWN) {
        lastIrreversiblePly[ply] = ply;
    } else {
        // keep the same last irreversible ply
        lastIrreversiblePly[ply] = lastIrreversiblePly[ply - 1];
    }
}

void Board::unmakeMove(const Move& move) {
    uint64_t oldSquare = move.getSource();
    uint64_t currSquare = move.getTarget();
    int moveFlag = move.getFlag();
    uint64_t capturedSquare = (moveFlag == ENPASSANT) ? (turn ? currSquare + 8 : currSquare - 8) : currSquare;

    uint64_t oldSquareMask = 1ULL << oldSquare;
    uint64_t currSquareMask = 1ULL << currSquare;
    uint64_t captureSquareMask = 1ULL << capturedSquare;

    // Update ply and last irreversible ply
    --ply;

    // Get old gamestate
    Gamestate gState = history.top();
    history.pop();
    Gamestate oldGamestate = history.top();

    int movedPiece = isPromotion[moveFlag] ? (turn ? BPAWN : WPAWN) : squares[currSquare];
    int capturedPiece = gState.capturedPiece;

    // Revert zobrist hash for en passant
    if (gState.enpassantColumn == -1) {
        zobristHash ^= zobristBitstrings[774]; // no enpassant column
    } else {
        zobristHash ^= zobristBitstrings[775 + gState.enpassantColumn];
    }
    if (oldGamestate.enpassantColumn == -1) {
        zobristHash ^= zobristBitstrings[774]; // no enpassant column
    } else {
        zobristHash ^= zobristBitstrings[775 + oldGamestate.enpassantColumn];
    }

    // Bring back moved piece to old square
    squares[oldSquare] = movedPiece;
    pieceBitboards[movedPiece] ^= oldSquareMask;
    zobristHash ^= zobristBitstrings[movedPiece * NUM_SQUARES + oldSquare];
    pieceSquareEvalMG += pieceSquareTablesMG[movedPiece][oldSquare];
    pieceSquareEvalEG += pieceSquareTablesEG[movedPiece][oldSquare];
    blockers ^= oldSquareMask;

    // Bring back captured piece
    squares[capturedSquare] = capturedPiece;
    if (capturedPiece != EMPTY) {
        pieceBitboards[capturedPiece] |= captureSquareMask; 
        zobristHash ^= zobristBitstrings[capturedPiece * NUM_SQUARES + capturedSquare];
        pieceSquareEvalMG += pieceSquareTablesMG[capturedPiece][capturedSquare];
        pieceSquareEvalEG += pieceSquareTablesEG[capturedPiece][capturedSquare];
        blockers ^= captureSquareMask;
    }

    // Flag specific cases
    if (moveFlag == NONE || moveFlag == PAWNTWOFORWARD || moveFlag == ENPASSANT || moveFlag == CASTLING) {
        // Remove moved piece from square that it was moved to
        pieceBitboards[movedPiece] ^= currSquareMask;
        zobristHash ^= zobristBitstrings[movedPiece * NUM_SQUARES + currSquare];
        pieceSquareEvalMG -= pieceSquareTablesMG[movedPiece][currSquare];
        pieceSquareEvalEG -= pieceSquareTablesEG[movedPiece][currSquare];
        blockers ^= currSquareMask;

        if (moveFlag == ENPASSANT) {
            squares[currSquare] = EMPTY;
        }
    } else if (isPromotion[moveFlag]) {
        // Remove promoted piece from square that it was promoted
        int promotedPiece = moveFlag - 2 + (turn ? 6 : 0);

        pieceBitboards[promotedPiece] ^= currSquareMask;
        zobristHash ^= zobristBitstrings[promotedPiece * NUM_SQUARES + currSquare];
        pieceSquareEvalMG -= pieceSquareTablesMG[promotedPiece][currSquare];
        pieceSquareEvalEG -= pieceSquareTablesEG[promotedPiece][currSquare];
        blockers ^= currSquareMask;
    }

    // In castling moves, the rook is moved along with the king
    if (moveFlag == CASTLING) {
        // Determine rook movement based on target square
        int rookFrom = -1, rookTo = -1;
        int rookPiece = (movedPiece < 6) ? WROOK : BROOK;

        if (currSquare == 1) { rookTo = 0; rookFrom = 2; }
        else if (currSquare == 5) { rookTo = 7; rookFrom = 4; }
        else if (currSquare == 57) { rookTo = 56; rookFrom = 58; }
        else if (currSquare == 61) { rookTo = 63; rookFrom = 60; }

        // Move rook
        if (rookFrom != -1 && rookTo != -1) {
            uint64_t rFromMask = 1ULL << rookFrom;
            uint64_t rToMask   = 1ULL << rookTo;

            squares[rookFrom] = EMPTY;
            pieceBitboards[rookPiece] ^= rFromMask;
            zobristHash ^= zobristBitstrings[rookPiece * NUM_SQUARES + rookFrom];
            pieceSquareEvalMG -= pieceSquareTablesMG[rookPiece][rookFrom];
            pieceSquareEvalEG -= pieceSquareTablesEG[rookPiece][rookFrom];
            blockers ^= rFromMask;

            squares[rookTo] = rookPiece;
            pieceBitboards[rookPiece] ^= rToMask;
            zobristHash ^= zobristBitstrings[rookPiece * NUM_SQUARES + rookTo];
            pieceSquareEvalMG += pieceSquareTablesMG[rookPiece][rookTo];
            pieceSquareEvalEG += pieceSquareTablesEG[rookPiece][rookTo];
            blockers ^= rToMask;
        }
    }

    // Update castling right zobrists
    uint8_t currRights = gState.castlingRights;
    uint8_t oldRights = oldGamestate.castlingRights;

    uint8_t changed = oldRights ^ currRights;
    if (changed) {
        static constexpr int zobristCastlingBase = 769;
        for (int i = 0; i < 4; ++i) {
            if (changed & (1u << i)) {
                zobristHash ^= zobristBitstrings[zobristCastlingBase + i];
            }
        }
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
    zobristHash = 0ULL;
    
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

    // 769 - 772: castling rights
    for (int i = 0; i < 4; i++) {
        if (history.top().castlingRights & (1u << i)) {
            zobristHash ^= zobristBitstrings[769 + i];
        }
    }

    // 773-782: en passant column (774 = no en passant, 775-782 = columns 0-7)
    if (history.top().enpassantColumn == -1) {
        zobristHash ^= zobristBitstrings[774]; // no enpassant column
    } else {
        zobristHash ^= zobristBitstrings[775 + history.top().enpassantColumn];
    }
}

bool Board::isThreeFoldRepetition() const {
    int count = 0;
    int end = lastIrreversiblePly[ply];

    if (ply - end < 4) return false;
    for (int i = ply - 2; i >= end; i -= 2) {
        if (zobristHistory[i] == zobristHash) {
            count += 1;
            if (count >= 2) {
                return true;
            }
        }
    }
    return false;
}

void Board::setPieceSquareEvaluation() {
    pieceSquareEvalMG = 0;
    pieceSquareEvalEG = 0;
    for (int i = 0; i < 64; i++) {
        if (squares[i] != EMPTY) {
            pieceSquareEvalMG += pieceSquareTablesMG[squares[i]][i];
            pieceSquareEvalEG += pieceSquareTablesEG[squares[i]][i];
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