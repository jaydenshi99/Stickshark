#pragma once

#include <iostream>
#include <cstdint>

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
    private:
    // Data
    uint16_t moveValue;

    public:
    // Constructors
    Move();
    Move(int s, int t, int f);

    // Get methods
    uint16_t getSource() const;
    uint16_t getTarget() const;
    uint16_t getFlag() const;
};