#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "../board/board.h"
#include "../../bit.h"
#include "../../utility.h"
#include "bitTables.h"
#include "../../engine/evalConstants.h"
#include "../../pool.h"

#define ATTACK_MODIFIER 1000

#define MAX_POOL_CAPACITY 100

struct MoveList {
    Move* moves;          // start of chunk
    std::ptrdiff_t count; // valid moves in each chunk
};

class MoveGen {
    private:
    Pool* pseudoMovesPool;
    Pool* legalMovesPool;
    
    // Storing data that is used accross all functions
    uint64_t friendly;
    uint64_t enemyOrEmpty;

    // Settings
    bool onlyGenerateForcing;

    // Constructor Destructor
    MoveGen();
    ~MoveGen();

    // These functions require the working of generatePseudoMoves
    void generatePawnMoves(const Board& b,  Move* &pseudoMoves);
    void generateKnightMoves(const Board& b, Move* &pseudoMoves);
    void generateSlidingMoves(const Board& b, Move* &pseudoMoves);
    void generateKingMoves(const Board& b, Move* &pseudoMoves);

    bool findLegalMoveToTarget(Board& b, int targetSquare, MoveList& pseudoMoves, Move& out);

    public:
    static MoveGen& getInstance();

    MoveGen(const MoveGen&) = delete;
    MoveGen& operator=(const MoveGen&) = delete;
    MoveGen(MoveGen&&) = delete;
    MoveGen& operator=(MoveGen&&) = delete;

    MoveList generatePseudoMoves(const Board& b, bool onlyGenerateForcing);
    MoveList generateLegalMoves(Board& b);
    void orderMoves(Board& b, MoveList& pseudoMoves, uint16_t bestMoveValue);
    void freePseudoMoves(MoveList& pseudoMoves);
    void freeLegalMoves(MoveList& legalMoves);

    // Move getLeastValuableAttack(Board& b, int targetSquare);
};