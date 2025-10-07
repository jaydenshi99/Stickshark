#include "transpositionTable.h"

using namespace std;

TranspositionTable::TranspositionTable() {
    clear();
}

TranspositionTable::~TranspositionTable() {
    clear();
}

void TranspositionTable::clear() {
    generation = 0;
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

    // priorities higher depth and newer entries
    if (currentEntry.depth > depth) {
        return;
    } else if (currentEntry.depth == depth && currentEntry.generation > generation) {
        return;
    }

    // Add the entry to the table
    table[key22] = TTEntry(key16, bestMove, evaluation, generation, depth, flag);
}

bool TranspositionTable::retrieveEntry(uint64_t zobristHash, TTEntry& entry) {
    uint64_t key22 = zobristHash >> (64 - 22);
    uint16_t key16 = zobristHash & 0xFFFF;

    entry = table[key22];
    if (generation != 0 && entry.key16 == key16) {
        return true;
    }

    return false;
}