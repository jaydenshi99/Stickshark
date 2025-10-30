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
    for (uint64_t i = 0; i < TABLE_SIZE; i++) {
        table[i] = TTEntry();
    }
}

void TranspositionTable::incrementGeneration() {
    generation++;
}

void TranspositionTable::addEntry(uint64_t zobristHash, uint16_t bestMove, int16_t score, uint8_t depth, uint8_t flag) {
    uint64_t slotIndex = (uint64_t)((zobristHash >> 42) & (TABLE_SIZE - 1));
    uint32_t key32 = (uint32_t)(zobristHash & 0xFFFFFFFFu);

    TTEntry currentEntry = table[slotIndex];

    uint8_t plyToMate = abs(score) > MATE - 100 ? MATE - abs(score) : 0;
    int16_t packedScore = score > MATE - 100 ? MATE : score < -MATE + 100 ? -MATE : score; // 32000 means mate

    // Replace if:
    // 1) Empty slot
    // 2) Same position: deeper depth OR same depth but newer/same generation
    // 3) Different position: newer generation OR same generation but deeper/equal depth

    bool empty = currentEntry.generation == 0; // treat never-written entries as empty
    bool samePos = currentEntry.key32 == key32;

    bool replace =
        empty
        || (samePos && (depth > currentEntry.depth ||
                        (depth == currentEntry.depth && generation >= currentEntry.generation)))
        || (!samePos && (generation > currentEntry.generation ||
                        (generation == currentEntry.generation && depth >= currentEntry.depth)));

    if (replace) {
        table[slotIndex] = TTEntry(key32, bestMove, packedScore, generation, depth, flag, plyToMate);

        if (empty) numFilledEntries++;
    }
}

bool TranspositionTable::retrieveEntry(uint64_t zobristHash, TTEntry& entry) {
    uint64_t slotIndex = (uint64_t)((zobristHash >> 42) & (TABLE_SIZE - 1));
    uint32_t key32 = (uint32_t)(zobristHash & 0xFFFFFFFFu);

    entry = table[slotIndex];
    if (entry.key32 == key32 && entry.generation != 0) {
        return true;
    }

    return false;
}

int TranspositionTable::getNumFilledEntries() {
    return numFilledEntries;
}