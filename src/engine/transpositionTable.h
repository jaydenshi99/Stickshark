#pragma once

#include "../chess/moveGeneration/moveGen.h"

#define EXACT 0
#define UPPERBOUND 1
#define LOWERBOUND 2

struct TTEntry {
    uint16_t key16; // lowest 16 bits
    uint16_t bestMove;
    int16_t evaluation;
    uint16_t generation;
    uint8_t depth;
    uint8_t flag; // EXACT, UPPERBOUND, LOWERBOUND
    
    TTEntry() = default;
    TTEntry(uint16_t k, uint16_t bm, int16_t eval, uint16_t gen, uint8_t d, uint8_t f) 
        : key16(k), bestMove(bm), evaluation(eval), generation(gen), depth(d), flag(f) {}
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