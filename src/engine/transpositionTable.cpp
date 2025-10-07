#include "transpositionTable.h"

using namespace std;

TranspositionTable::TranspositionTable() {
    table = new TTEntry[TABLE_SIZE];
    clear();
}

TranspositionTable::~TranspositionTable() {
    delete[] table;
}

void TranspositionTable::clear() {
    generation = 1;
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = TTEntry();
    }
}

void TranspositionTable::incrementGeneration() {
    generation++;
}

void TranspositionTable::addEntry(uint64_t zobristHash, uint16_t bestMove, int16_t evaluation, uint8_t depth, uint8_t flag) {
    uint64_t key22 = zobristHash >> (64 - 22);
    uint16_t key16 = zobristHash & 0xFFFF;

    TTEntry currentEntry = table[key22];

    // Replace if:
    // 1. Empty slot (key16 == 0)
    // 2. Same position but deeper/newer search
    // 3. Different position but newer generation

    bool empty = currentEntry.key16 == 0;
    bool samePosition = currentEntry.key16 == key16 && ( depth > currentEntry.depth || 
        (depth == currentEntry.depth && generation > currentEntry.generation));
    bool diffPosition = currentEntry.key16 != key16 && generation > currentEntry.generation;

    if (empty || samePosition || diffPosition) {
        table[key22] = TTEntry(key16, bestMove, evaluation, generation, depth, flag);
    }
}

bool TranspositionTable::retrieveEntry(uint64_t zobristHash, TTEntry& entry) {
    uint64_t key22 = zobristHash >> (64 - 22);
    uint16_t key16 = zobristHash & 0xFFFF;

    entry = table[key22];
    if (entry.key16 == key16 && entry.generation != 0) {
        return true;
    }

    return false;
}