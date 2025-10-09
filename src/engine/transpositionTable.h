#pragma once

#include "../chess/moveGeneration/moveGen.h"

#define EXACT 0
#define UPPERBOUND 1
#define LOWERBOUND 2

struct TTEntry {
    uint32_t key32 = 0;       // lowest 32 bits
    uint16_t bestMove = 0;
    int16_t evaluation = 0;
    uint16_t generation = 0;
    uint8_t depth = 0;
    uint8_t flag = 0;         // EXACT, UPPERBOUND, LOWERBOUND
    
    TTEntry() = default; // value-initialized to zeros via member initializers
    TTEntry(uint32_t k, uint16_t bm, int16_t eval, uint16_t gen, uint8_t d, uint8_t f) 
        : key32(k), bestMove(bm), evaluation(eval), generation(gen), depth(d), flag(f) {}
};

class TranspositionTable {
    private:
    const static int TABLE_SIZE = (1u << 22); // 2^22 = 4,194,304 entries (~50MB)
    uint16_t generation = 1;

    TTEntry* table;

    public:
    TranspositionTable();
    ~TranspositionTable();

    inline void getGeneration(uint16_t& gen) const {
        gen = generation;
    }

    void clear();
    void incrementGeneration();
    void addEntry(uint64_t zobristHash, uint16_t bestMove, int16_t evaluation, uint8_t depth, uint8_t flag);
    bool retrieveEntry(uint64_t zobristHash, TTEntry& entry);
};