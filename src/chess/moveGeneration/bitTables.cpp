#include "bitTables.h"

using namespace std;

const bool isNonSliding[12] = {
    true, false, true, false, false, true, 
    true, false, true, false, false, true
};

const bool isPromotion[8] {
    false, false, false, true, true, true, true, false
};

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

uint64_t bishopAttackBitboards[BISHOP_ATTACK_TABLE_SIZE];
uint64_t rookAttackBitboards[ROOK_ATTACK_TABLE_SIZE];

uint64_t kingAttackBitboards[NUM_SQUARES];

uint64_t bishopAttackMagicMasks[NUM_SQUARES];
uint64_t rookAttackMagicMasks[NUM_SQUARES];
uint64_t bishopMagics[NUM_SQUARES];
uint64_t rookMagics[NUM_SQUARES];

int chebyshevDistances[NUM_SQUARES][NUM_SQUARES];
int manhattanDistances[NUM_SQUARES][NUM_SQUARES];
int centralManhattanDistances[NUM_SQUARES];

uint64_t kingPawnShieldMasks[NUM_SQUARES * 2];
uint64_t kingZoneMasks[NUM_SQUARES * 2];


uint64_t zobristBitstrings[783];

void computeAllTables() {
    auto start = chrono::high_resolution_clock::now();

    computeNotBitboards();
    cout << "File and rank bitboards computed" << endl;

    computeKnightAttacks();
    cout << "Knight attacks computed" << endl;

    computeSlidingAttacks();
    cout << "Sliding attacks computed" << endl;

    computeKingAttacks();
    cout << "King attacks computed" << endl;

    computeZobristBitstrings();
    cout << "Zobrist bitstrings computed" << endl;

    calculatePieceSquareTables();
    cout << "Piece square tables computed" << endl;

    calculateKingZoneAttackPenalty();
    cout << "King zone attack penalty computed" << endl;

    computeManhattanDistances();
    cout << "Chebyshev distances computed" << endl;

    computeManhattanDistances();
    cout << "Manhattan distances computed" << endl;

    computeKingPawnShieldMasks();
    cout << "King pawn shield masks computed" << endl;

    computeKingZoneMasks();
    cout << "King zone masks computed" << endl;

    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Time taken: " << duration.count() << " ms" << endl << endl;
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

void computeSlidingAttacks() {
    // Load and calculate prerequisites
    loadMagics();
    cout << "Magic numbers loaded" << endl;

    computeMagicAttackMasks();
    cout << "Magic attack masks computed" << endl;

    // Compute bishop attacks
    for (int square = 0; square < NUM_SQUARES; square++) {
        uint64_t attackMask = bishopAttackMagicMasks[square];
        vector<uint64_t> occupancies = generateAllOccupancies(attackMask);

        for (uint64_t occupancy : occupancies) {
            uint64_t attackBitboard = computeBishopAttackBitboard(square, occupancy);
            int index = square * BISHOP_ATTACKS_PER_SQUARE + 
            ((occupancy * bishopMagics[square]) >> 55);
            
            bishopAttackBitboards[index] = attackBitboard;
        }
    }

    // Compute rook attacks
    for (int square = 0; square < NUM_SQUARES; square++) {
        uint64_t attackMask = rookAttackMagicMasks[square];
        vector<uint64_t> occupancies = generateAllOccupancies(attackMask);

        for (uint64_t occupancy : occupancies) {
            uint64_t attackBitboard = computeRookAttackBitboard(square, occupancy);
            int index = square * ROOK_ATTACKS_PER_SQUARE + 
            ((occupancy * rookMagics[square]) >> 52);
            
            rookAttackBitboards[index] = attackBitboard;
        }
    }
}

void computeKingAttacks() {
    for (int i = 0; i < NUM_SQUARES; i++) {
        uint64_t kingPos = 1ULL << i;
        kingAttackBitboards[i] = kingPos << 8;
        kingAttackBitboards[i] |= (kingPos << 7) & notFileBitboards[0];
        kingAttackBitboards[i] |= (kingPos >> 1) & notFileBitboards[0];
        kingAttackBitboards[i] |= (kingPos >> 9) & notFileBitboards[0];
        kingAttackBitboards[i] |= kingPos >> 8;
        kingAttackBitboards[i] |= (kingPos >> 7) & notFileBitboards[7];
        kingAttackBitboards[i] |= (kingPos << 1) & notFileBitboards[7];
        kingAttackBitboards[i] |= (kingPos << 9) & notFileBitboards[7];
    }
}

uint64_t computeBishopAttackBitboard(int square, uint64_t blockers) {
    uint64_t attackBitboard = 0;

    // NE
    for (int i = square + 7; i % 8 < 7 && i % 8 >= 0 && i < 64; i += 7) {
        attackBitboard |= 1ULL << i;
        if (((1ULL << i) & blockers) != 0) {
            break;
        }
    }

    // NW
    for (int i = square + 9; i % 8 <= 7 && i % 8 > 0 && i < 64; i += 9) {
        attackBitboard |= 1ULL << i;
        if (((1ULL << i) & blockers) != 0) {
            break;
        }
    }

    // SE
    for (int i = square - 9; i % 8 < 7 && i % 8 >= 0 && i >= 0; i -= 9) {
        attackBitboard |= 1ULL << i;
        if (((1ULL << i) & blockers) != 0) {
            break;
        }
    }

    // SW
    for (int i = square - 7; i % 8 <= 7 && i % 8 > 0 && i >= 0; i -= 7) {
        attackBitboard |= 1ULL << i;
        if (((1ULL << i) & blockers) != 0) {
            break;
        }
    }

    return attackBitboard;
}

uint64_t computeRookAttackBitboard(int square, uint64_t blockers) {
    uint64_t attackBitboard = 0;

    // Up
    for (int i = square + 8; i < 64; i += 8) {
        attackBitboard |= 1ULL << i;
        if (((1ULL << i) & blockers) != 0) {
            break;
        }
    }

    // Down
    for (int i = square - 8; i >= 0; i -= 8) {
        attackBitboard |= 1ULL << i;
        if (((1ULL << i) & blockers) != 0) {
            break;
        }
    }

    // Left
    for (int i = square + 1; i % 8 <= 7 && i / 8 == square / 8; i += 1) {
        attackBitboard |= 1ULL << i;
        if (((1ULL << i) & blockers) != 0) {
            break;
        }
    }

    // Right
    for (int i = square - 1; i % 8 >= 0 && i / 8 == square / 8; i -= 1) {
        attackBitboard |= 1ULL << i;
        if (((1ULL << i) & blockers) != 0) {
            break;
        }
    }

    return attackBitboard;
}

// Magics
void computeMagics() {
    computeMagicAttackMasks();

    srand(static_cast<unsigned>(time(0)));

    // Generate Rook Magics
    cout << "Finding rook magics" << endl;
    for (int i = 0; i < 64; i++) {
        uint64_t magic = generateRandomMagic();
        while (!testMagic(magic, rookAttackMagicMasks[i], false)) {
            magic = generateRandomMagic();
        }
        cout << "Found " << i << ": " << magic << endl;;
        rookMagics[i] = magic;
    }

    // Generate Bishop Magics
    cout << endl << "Finding bishop magics" << endl;
    for (int i = 0; i < 64; i++) {
        uint64_t magic = generateRandomMagic();
        while (!testMagic(magic, bishopAttackMagicMasks[i], true)) {
            magic = generateRandomMagic();
        }
        cout << "Found " << i << ": " << magic << endl;;
        bishopMagics[i] = magic;
    }

    saveMagics();
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

// Generates sparse random magics
uint64_t generateRandomMagic() {
    uint64_t magic = 0;
    int numSetBits = std::rand() % 10 + 1;

    for (int i = 0; i < numSetBits; ++i) {
        int bitPosition;
        do {
            bitPosition = std::rand() % 64; 
        } while (magic & (1ULL << bitPosition)); 

        magic |= (1ULL << bitPosition); 
    }

    return magic;
}

bool testMagic(uint64_t magic, uint64_t mask, bool isBishop) {
    unordered_set<uint64_t> indices;
    int shiftAmt = isBishop ? 55 : 52;

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
        setBits.push_back(popLSB(mask));
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

void loadMagics() {
    ifstream inFile("data/magics.txt");

    if (!inFile) {
        cerr << "Error opening file for reading\n";
        exit(1);
    }

    int lineNum = 0;
    string line;
    while (getline(inFile, line) && lineNum < 128) {
        try {
            if (lineNum >= 64) {
                rookMagics[lineNum - 64] = stoull(line);
            } else {
                bishopMagics[lineNum] = stoull(line);
            }
        } catch (const exception& e) {
            cerr << "Error parsing line\n";
        }

        lineNum++;
    }

    inFile.close();
}

void saveMagics() {
    ofstream outFile("data/magics.txt");

    if (!outFile) {
        cerr << "Error opening file for writing\n";
        exit(1);
    }

    for (int i = 0; i < 64; i++) {
        outFile << bishopMagics[i] << endl;
    }
    
    for (int i = 0; i < 64; i++) {
        outFile << rookMagics[i] << endl;
    }

    outFile.close();
}

void computeZobristBitstrings() {
    for (int i = 0; i < 783; i++) {
        zobristBitstrings[i] = random_uint64();
    }
}

void computeChebyshevDistances() {
    for (int i = 0; i < NUM_SQUARES; i++) {
        for (int j = 0; j < NUM_SQUARES; j++) {
            chebyshevDistances[i][j] = max(abs(i % 8 - j % 8), abs(i / 8 - j / 8));
        }
    }
}

void computeManhattanDistances() {
    for (int i = 0; i < NUM_SQUARES; i++) {
        for (int j = 0; j < NUM_SQUARES; j++) {
            manhattanDistances[i][j] = abs(i % 8 - j % 8) + abs(i / 8 - j / 8);
        }
    }

    for (int i = 0; i < NUM_SQUARES; i++) {
        centralManhattanDistances[i] = manhattanDistances[i][27];
        centralManhattanDistances[i] = min(centralManhattanDistances[i], manhattanDistances[i][28]);
        centralManhattanDistances[i] = min(centralManhattanDistances[i], manhattanDistances[i][35]);
        centralManhattanDistances[i] = min(centralManhattanDistances[i], manhattanDistances[i][36]);
    }
}

void computeKingPawnShieldMasks() {
    for (int i = 0; i < NUM_SQUARES; i++) {
        // white
        kingPawnShieldMasks[i] |= 1ULL << (i + 7) & notFileBitboards[0];
        kingPawnShieldMasks[i] |= 1ULL << (i + 8);
        kingPawnShieldMasks[i] |= 1ULL << (i + 9) & notFileBitboards[7];

        // black
        kingPawnShieldMasks[i + NUM_SQUARES] |= 1ULL << (i - 7) & notFileBitboards[7];
        kingPawnShieldMasks[i + NUM_SQUARES] |= 1ULL << (i - 8);
        kingPawnShieldMasks[i + NUM_SQUARES] |= 1ULL << (i - 9) & notFileBitboards[0];
    }
}

void computeKingZoneMasks() {
    for (int i = 0; i < NUM_SQUARES; i++) {
        kingZoneMasks[i] |= kingAttackBitboards[i];
        kingZoneMasks[i + 64] |= kingAttackBitboards[i];

        if (i + 8 <= 64) {
            kingZoneMasks[i] |= kingAttackBitboards[i + 8];
        } 

        if (i - 8 >= 0) {
            kingZoneMasks[i + 64] |= kingAttackBitboards[i - 8];
        }
    }
}