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
    uint64_t key22 = (uint64_t)((zobristHash >> 42) & ((1ull << 22) - 1));
    uint32_t key32 = (uint32_t)(zobristHash & 0xFFFFFFFFu);

    TTEntry currentEntry = table[key22];

    // Replace if:
    // 1) Empty slot
    // 2) Same position: deeper depth OR same depth but newer/same generation
    // 3) Different position: newer generation OR same generation but deeper/equal depth

    bool empty = currentEntry.key32 == 0;
    bool samePos = currentEntry.key32 == key32;

    bool replace =
        empty
        || (samePos && (depth > currentEntry.depth ||
                        (depth == currentEntry.depth && generation >= currentEntry.generation)))
        || (!samePos && (generation > currentEntry.generation ||
                        (generation == currentEntry.generation && depth >= currentEntry.depth)));

    if (replace) {
        table[key22] = TTEntry(key32, bestMove, evaluation, generation, depth, flag);
    }
}

bool TranspositionTable::retrieveEntry(uint64_t zobristHash, TTEntry& entry) {
    uint64_t key22 = (uint64_t)((zobristHash >> 42) & ((1ull << 22) - 1));
    uint32_t key32 = (uint32_t)(zobristHash & 0xFFFFFFFFu);

    entry = table[key22];
    if (entry.key32 == key32 && entry.generation != 0) {
        return true;
    }

    return false;
}