#include "move.h"

using namespace std;

Move::Move() {
    moveScore = 0;
    moveValue = 0;
}

Move::Move(int s, int t, int f) {
    moveScore = 0;
    moveValue = (f << 12) | (s << 6) | t;
}

uint16_t Move::getSource() const {
    return (moveValue >> 6) & 0x3F;
}

uint16_t Move::getTarget() const {
    return moveValue & 0x3F;
}

uint16_t Move::getFlag() const {
    return (moveValue >> 12) & 0xF;
}

ostream& operator<<(std::ostream& os, const Move& move) {
    if (move.getSource() == 0 && move.getTarget() == 0) {
        os << "Invalid Move";
    } else {
        os << moveToNotation(move.getSource()) << "-" << moveToNotation(move.getTarget());
    }
    return os;
}

string moveToNotation(int square) {
    char file = 'h' - (square % 8);
    char rank = '1' + (square / 8);
    return string(1, file) + rank;
}