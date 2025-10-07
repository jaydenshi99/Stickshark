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
    generation = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = TTEntry();
    }
}

void TranspositionTable::incrementGeneration() {
    generation++;
}

void TranspositionTable::addEntry(uint64_t zobristHash, uint16_t bestMove, int16_t evaluation, uint8_t depth, uint8_t flag) {
    uint64_t tkey16 = zobristHash >> (64 - 16);
    uint16_t key16 = zobristHash & 0xFFFF;

    TTEntry currentEntry = table[tkey16];

    // priorities higher depth and newer entries
    if (currentEntry.depth > depth) {
        return;
    } else if (currentEntry.depth == depth && currentEntry.generation > generation) {
        return;
    }

    // Add the entry to the table
    table[tkey16] = TTEntry(key16, bestMove, evaluation, generation, depth, flag);
}

bool TranspositionTable::retrieveEntry(uint64_t zobristHash, TTEntry& entry) {
    uint64_t tkey16 = zobristHash >> (64 - 16);
    uint16_t key16 = zobristHash & 0xFFFF;

    entry = table[tkey16];
    if (generation != 0 && entry.key16 == key16) {
        return true;
    }

    return false;
}