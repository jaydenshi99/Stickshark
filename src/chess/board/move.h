#pragma once

#include <iostream>
#include <cstdint>
#include <string>

#define STARTMASK  0x03F0
#define TARGETMASK 0x003F
#define FLAGMASK   0xF000

// Flags
#define NONE 0
#define ENPASSANT 1
#define CASTLING 2
#define PROMOTEBISHOP 3
#define PROMOTEKNIGHT 4
#define PROMOTEROOK 5
#define PROMOTEQUEEN 6
#define PAWNTWOFORWARD 7

class Move {
    public:
    uint16_t moveValue;

    int moveScore;

    // Constructors
    Move();
    Move(uint16_t mv);
    Move(int s, int t, int f);

    // Get methods (inlined for hot-path performance)
    inline __attribute__((always_inline)) uint16_t getSource() const { return (moveValue >> 6) & 0x3F; }
    inline __attribute__((always_inline)) uint16_t getTarget() const { return moveValue & 0x3F; }
    inline __attribute__((always_inline)) uint16_t getFlag()   const { return (moveValue >> 12) & 0xF; }

    // Friend declaration for operator<<
    friend std::ostream& operator<<(std::ostream& os, const Move& move);
};

std::string moveToNotation(int square);