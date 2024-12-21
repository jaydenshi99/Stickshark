#include "bit.h"

inline int popLSB(uint64_t *bb) {
    int lsb = __builtin_ctzll(*bb);
    *bb &= *bb - 1; 
    return lsb;    
}