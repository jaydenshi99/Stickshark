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
    Move(int s, int t, int f);

    // Get methods
    uint16_t getSource() const;
    uint16_t getTarget() const;
    uint16_t getFlag() const;

    // Friend declaration for operator<<
    friend std::ostream& operator<<(std::ostream& os, const Move& move);
};

std::string moveToNotation(int square);