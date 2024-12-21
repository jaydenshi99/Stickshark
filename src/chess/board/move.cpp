#include "move.h"

using namespace std;

Move::Move(int s, int t, int f) {
    moveValue = (f << 12) | (s << 6) | t;
}

uint16_t Move::getStart() const {
    return (moveValue >> 6) & 0x3F;
}

uint16_t Move::getTarget() const {
    return moveValue & 0x3F;
}

uint16_t Move::getFlag() const {
    return (moveValue >> 12) & 0xF;
}