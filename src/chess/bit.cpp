#include "bit.h"

int popLSB(uint64_t& bb) {
    int lsb = __builtin_ctzll(bb);
    bb &= bb - 1; 
    return lsb;    
}

uint64_t random_uint64() {
    return (static_cast<uint64_t>(std::rand()) & 0xFFFF)
         | ((static_cast<uint64_t>(std::rand()) & 0xFFFF) << 16)
         | ((static_cast<uint64_t>(std::rand()) & 0xFFFF) << 32)
         | ((static_cast<uint64_t>(std::rand()) & 0xFFFF) << 48);
}