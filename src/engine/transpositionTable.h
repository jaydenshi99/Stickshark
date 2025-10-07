#pragma once

#include "../chess/moveGeneration/moveGen.h"

class TranspositionTable {
    private:
    const static int TABLE_SIZE = (1u << 22); // 2^22
    uint16_t generation = 1;

    TTEntry table[TABLE_SIZE];

    public:
    TranspositionTable();
    ~TranspositionTable();

    void clear();
    void incrementGeneration();
    void addEntry(uint64_t zobristHash, uint16_t bestMove, int16_t evaluation, uint8_t depth, uint8_t flag);
    bool retrieveEntry(uint64_t zobristHash, TTEntry& entry);
};

struct TTEntry {
    uint16_t key16; // lowest 16 bits
    uint16_t bestMove;
    int16_t evaluation;
    uint16_t generation;
    uint8_t depth;
    uint8_t flag; // EXACT, UPPERBOUND, LOWERBOUND
};