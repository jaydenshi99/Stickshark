#pragma once

#include <cstdint>
#include <cstdlib>
#include <random>


inline int popLSB(uint64_t& bb) {
    int lsb = __builtin_ctzll(bb);
    bb &= bb - 1; 
    return lsb;    
}

inline int popcount(uint64_t bb) {
    return __builtin_popcountll(bb);
}

uint64_t random_uint64();