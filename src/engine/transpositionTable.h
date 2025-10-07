#pragma once

#include "../chess/moveGeneration/moveGen.h"

class TranspositionTable {
    private:
    const static int TABLE_SIZE = 1024 * 1024 * 4; // 2^22
    uint16_t generation = 0;

    TTEntry table[TABLE_SIZE];

    public:
    TranspositionTable();
    ~TranspositionTable();

    void clear();
    void incrementGeneration();
    void addEntry(uint16_t key16, uint16_t bestMove, uint16_t evaluation, uint8_t depth, uint8_t flag);
    bool retrieveEntry(uint16_t key16, TTEntry& entry);
};

struct TTEntry {
    uint16_t key16; // lowest 16 bits
    uint16_t bestMove;
    uint16_t evaluation;
    uint16_t generation;
    uint8_t depth;
    uint8_t flag; // EXACT, UPPERBOUND, LOWERBOUND
};