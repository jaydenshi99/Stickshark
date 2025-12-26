#include "transpositionTable.h"

using namespace std;

TranspositionTable::TranspositionTable() {
    table = new TTEntry[NUM_BUCKETS * BUCKET_SIZE];
    clear();
}

TranspositionTable::~TranspositionTable() {
    delete[] table;
}

void TranspositionTable::clear() {
    generation = 1;
    for (uint64_t i = 0; i < NUM_BUCKETS * BUCKET_SIZE; i++) {
        table[i] = TTEntry();
    }

    numFilledEntries = 0;
}

void TranspositionTable::incrementGeneration() {
    generation++;
}

void TranspositionTable::addEntry(uint64_t zobristHash, uint16_t bestMove, int16_t score, uint8_t depth, uint8_t flag) {
    uint64_t bucketIndex = (uint64_t)((zobristHash >> 41) & (NUM_BUCKETS - 1));
    uint32_t key32 = (uint32_t)(zobristHash & 0xFFFFFFFFu);

    // Pack mate scores: store as distance from current position (ply-independent)
    uint8_t plyToMate = 0;
    int16_t packedScore = score;
    if (score > MATE - 100) {
        plyToMate = (MATE - score);
        packedScore = MATE;
    } else if (score < -MATE + 100) {
        plyToMate = (MATE + score);
        packedScore = -MATE;
    }

    TTEntry newEntry = TTEntry(key32, bestMove, packedScore, generation, depth, flag, plyToMate);

    // Check both slots in the bucket
    uint64_t slot0 = bucketIndex * BUCKET_SIZE;
    uint64_t slot1 = bucketIndex * BUCKET_SIZE + 1;

    TTEntry& entry0 = table[slot0];
    TTEntry& entry1 = table[slot1];

    // Replacement strategy for 2-way set associative:
    // 1) If exact match found in either slot, always replace it
    // 2) If empty slot exists, use it
    // 3) Otherwise, replace the entry with lower value (prefer deeper searches and newer generations)

    bool slot0Empty = entry0.generation == 0;
    bool slot1Empty = entry1.generation == 0;
    bool slot0SamePos = entry0.key32 == key32;
    bool slot1SamePos = entry1.key32 == key32;

    // Always replace exact match
    if (slot0SamePos) {
        if (slot0Empty) numFilledEntries++;
        table[slot0] = newEntry;
        return;
    }
    if (slot1SamePos) {
        if (slot1Empty) numFilledEntries++;
        table[slot1] = newEntry;
        return;
    }

    // Use empty slot if available
    if (slot0Empty) {
        numFilledEntries++;
        table[slot0] = newEntry;
        return;
    }
    if (slot1Empty) {
        numFilledEntries++;
        table[slot1] = newEntry;
        return;
    }

    // Both slots occupied with different positions - choose which to replace
    // Prefer replacing: shallower depth, tie-break with older generation
    if (entry0.depth < entry1.depth ||
        (entry0.depth == entry1.depth && entry0.generation < entry1.generation)) {
        table[slot0] = newEntry;
    } else {
        table[slot1] = newEntry;
    }
}

bool TranspositionTable::retrieveEntry(uint64_t zobristHash, TTEntry& entry) {
    uint64_t bucketIndex = (uint64_t)((zobristHash >> 41) & (NUM_BUCKETS - 1));
    uint32_t key32 = (uint32_t)(zobristHash & 0xFFFFFFFFu);

    // Check both slots in the bucket
    uint64_t slot0 = bucketIndex * BUCKET_SIZE;
    uint64_t slot1 = bucketIndex * BUCKET_SIZE + 1;

    bool slot0Match = table[slot0].key32 == key32 && table[slot0].generation != 0;
    bool slot1Match = table[slot1].key32 == key32 && table[slot1].generation != 0;

    // If both match, return the better one (deeper search, tie-break with newer generation)
    if (slot0Match && slot1Match) {
        if (table[slot0].depth > table[slot1].depth ||
            (table[slot0].depth == table[slot1].depth && table[slot0].generation >= table[slot1].generation)) {
            entry = table[slot0];
        } else {
            entry = table[slot1];
        }
        return true;
    }

    // Only one match
    if (slot0Match) {
        entry = table[slot0];
        return true;
    }
    if (slot1Match) {
        entry = table[slot1];
        return true;
    }

    return false;
}

int TranspositionTable::getNumFilledEntries() {
    return numFilledEntries;
}