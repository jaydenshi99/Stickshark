#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "../board/board.h"
#include "../../bit.h"
#include "../../utility.h"
#include "bitTables.h"
#include "../../engine/evalConstants.h"

#define ATTACK_MODIFIER 1000

class MoveGen {
    private:
    // Storing data that is used accross all functions
    uint64_t friendly;
    uint64_t enemyOrEmpty;

    // These functions require the working of generatePseudoMoves
    void generatePawnMoves(const Board& b);
    void generateKnightMoves(const Board& b);
    void generateSlidingMoves(const Board& b);
    void generateKingMoves(const Board& b);

    bool findLegalMoveToTarget(Board& b, int targetSquare, Move& out);

    public:
    
    bool onlyGenerateForcing;

    std::vector<Move> pseudoMoves;
    std::vector<Move> legalMoves;

    // Constructor
    MoveGen();

    void generatePseudoMoves(const Board& b);
    void generateLegalMoves(Board& b); // Does not have to be optimised as not used in search
    Move getLeastValuableAttack(Board& b, int targetSquare);
    void orderMoves(Board& b, uint16_t bestMoveValue);

    void clearMoves();

    struct LegalMoveDTO {
        uint16_t id;
        std::string from;
        std::string to;
        int flag;
    };

    void getLegalMovesDTO(Board& b, std::vector<LegalMoveDTO>& out);
};