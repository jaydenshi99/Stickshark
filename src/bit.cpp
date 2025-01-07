#include "bit.h"

uint64_t random_uint64() {
    static std::mt19937_64 rng(std::random_device{}());
    
    return rng();
}