#pragma once

#include <cstdint>
#include <string>

#include "../nn/encoder.h"

// Training data file: uint32 magic 0x50535353 ("SSSP"), int32 version = 1,
// then packed TrainingRecord entries (104 bytes each). Every field is
// side-to-move oriented (see nn::CompactPosition).
#pragma pack(push, 1)
struct TrainingRecord {
    nn::CompactPosition pos;
    int16_t policyIndex;   // move the engine played (nn::policyIndex)
    int16_t scoreCp;       // engine search score, side-to-move POV
    int8_t  result;        // final game result from side to move: +1 / 0 / -1
    uint8_t pad;
};
#pragma pack(pop)

struct SelfPlayConfig {
    int games = 100;
    std::string outPath = "data/selfplay.bin";
    uint64_t seed = 1;
    int moveTimeMs = 10;      // engine think time per move

    // knobs
    int maxBookPlies = 24;     // book walk depth is uniform in [0, maxBookPlies]
    int minRandomPlies = 2;    // uniform random plies after the book exit
    int maxRandomPlies = 6;
    float epsilonNoise = 0.07f;   // chance of a random (unlogged) move
    int noiseCutoffPly = 40;      // no noise injected after this ply
    int adjudicateCp = 800;       // |eval| for a win adjudication...
    int adjudicateMoves = 6;      // ...held for this many consecutive engine moves
    int maxPlies = 300;           // then adjudicate by last score (+/-200cp, else draw)
};

void runSelfPlay(const SelfPlayConfig& cfg);
