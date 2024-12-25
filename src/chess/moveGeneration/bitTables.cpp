#include "bitTables.h"

using namespace std;

const uint64_t fileBitboards[NUM_FILES] = {
    0x8080808080808080,
    0x4040404040404040,
    0x2020202020202020,
    0x1010101010101010,
    0x0808080808080808,
    0x0404040404040404,
    0x0202020202020202,
    0x0101010101010101
};

const uint64_t rankBitboards[NUM_RANKS] = {
    0x00000000000000FF,
    0x000000000000FF00,
    0x0000000000FF0000,
    0x00000000FF000000,
    0x000000FF00000000,
    0x0000FF0000000000,
    0x00FF000000000000,
    0xFF00000000000000
};

uint64_t notFileBitboards[NUM_FILES];
uint64_t notRankBitboards[NUM_RANKS];

uint64_t knightAttackBitboards[NUM_SQUARES];

uint64_t bishopAttackMagicMasks[NUM_SQUARES];
uint64_t rookAttackMagicMasks[NUM_SQUARES];
uint64_t bishopMagics[NUM_SQUARES];
uint64_t rookMagics[NUM_SQUARES];


void computeAllTables() {
    computeNotBitboards();
    computeKnightAttacks();
    computeMagicAttackMasks();
}

void computeNotBitboards() {
    for (int i = 0; i < NUM_FILES; i++) {
        notFileBitboards[i] = ~fileBitboards[i];
        notRankBitboards[i] = ~rankBitboards[i];
    }
}

void computeKnightAttacks() {
    for (int i = 0; i < NUM_SQUARES; i++) {
        uint64_t knightPos = 1ULL << i;
        knightAttackBitboards[i] = (knightPos << 15) & notFileBitboards[0] & notRankBitboards[0] & notRankBitboards[1];
        knightAttackBitboards[i] |= (knightPos << 6) & notFileBitboards[0] & notFileBitboards[1] & notRankBitboards[0];
        knightAttackBitboards[i] |= (knightPos >> 10) & notFileBitboards[0] & notFileBitboards[1] & notRankBitboards[7];
        knightAttackBitboards[i] |= (knightPos >> 17) & notFileBitboards[0] & notRankBitboards[6] & notRankBitboards[7];
        knightAttackBitboards[i] |= (knightPos >> 15) & notFileBitboards[7] & notRankBitboards[6] & notRankBitboards[7];
        knightAttackBitboards[i] |= (knightPos >> 6) & notFileBitboards[6] & notFileBitboards[7] & notRankBitboards[7];
        knightAttackBitboards[i] |= (knightPos << 10) & notFileBitboards[6] & notFileBitboards[7] & notRankBitboards[0];
        knightAttackBitboards[i] |= (knightPos << 17) & notFileBitboards[7] & notRankBitboards[0] & notRankBitboards[1];
    }
}

void computeMagicAttackMasks() {
    // Compute Rook Attack Masks
    for (int square = 0; square < NUM_SQUARES; square++) {
        rookAttackMagicMasks[square] = 0ULL;

        // Vertical
        int bottomSquare = square % 8 + 8;
        for (int i = bottomSquare; i < 56; i += 8) {
            rookAttackMagicMasks[square] |= 1ULL << i;
        }

        // Horizontal
        int rightSquare = square - square % 8 + 1;
        for (int i = rightSquare; i % 8 < 7; i += 1) {
            rookAttackMagicMasks[square] |= 1ULL << i;
        }

        // Empty rook square
        rookAttackMagicMasks[square] &= ~(1ULL << square);
    }

    // Compute Bishop Attack Masks
    for (int square = 0; square < NUM_SQUARES; square++) {
        bishopAttackMagicMasks[square] = 0ULL;

        // NE
        for (int i = square + 7; i % 8 < 7 && i % 8 > 0 && i < 56; i += 7) {
            bishopAttackMagicMasks[square] |= 1ULL << i;
        }

        // NW
        for (int i = square + 9; i % 8 < 7 && i % 8 > 0 && i < 56; i += 9) {
            bishopAttackMagicMasks[square] |= 1ULL << i;
        }

        // SE
        for (int i = square - 9; i % 8 < 7 && i % 8 > 0 && i > 7; i -= 9) {
            bishopAttackMagicMasks[square] |= 1ULL << i;
        }

        // SW
        for (int i = square - 7; i % 8 < 7 && i % 8 > 0 && i > 7; i -= 7) {
            bishopAttackMagicMasks[square] |= 1ULL << i;
        }
    }
}

void computeMagics() {

}

uint64_t generateRandomMagic() {
    uint64_t magic = random_uint64();

    for (int i = 0; i < 32; ++i) {
        if (std::rand() % 2 == 0) {
            magic &= ~(1ULL << (std::rand() % 64));
        }
    }

    return magic;
}

bool testMagic(uint64_t magic, uint64_t mask, bool isBishop) {
    unordered_set<uint64_t> indices;
    int shiftAmt = isBishop ? 55 : 50;

    vector<uint64_t> occupancies = generateAllOccupancies(mask);
    for (uint64_t occupancy : occupancies) {
        uint64_t index = (occupancy * magic) >> shiftAmt;
        indices.insert(index);
    }

    return indices.size() == occupancies.size();
}


vector<uint64_t> generateAllOccupancies(uint64_t mask) {
    vector<uint64_t> occupancies;

    int numBits = __libcpp_popcount(mask);
    
    vector<int> setBits;
    while (mask != 0) {
        setBits.push_back(popLSB(&mask));
    }

    for (uint64_t i = 0; i < pow(2, numBits); i++) {
        uint64_t occupancy = 0;
        for (int j = 0; j < numBits; j++) {
            if ((i & (1ULL << j)) != 0) {
                occupancy |= 1ULL << setBits[j];
            }
        }

        occupancies.push_back(occupancy);
    }

    return occupancies;
}