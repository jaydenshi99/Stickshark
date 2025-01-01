#pragma once

#include <cstdint>
#include <cstdlib>

inline int popLSB(uint64_t& bb) {
    int lsb = __builtin_ctzll(bb);
    bb &= bb - 1; 
    return lsb;    
}

uint64_t random_uint64();