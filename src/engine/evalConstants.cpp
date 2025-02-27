#include "evalConstants.h"

// King eval should never be used
const int materialEvaluations[NUM_PIECES] = {
    100, 330, 320, 500, 900, 20000, 
    -100, -330, -320, -500, -900, -20000
};

// Absolute value version of material evaluations.
const int moveScoreMaterialEvaluations[NUM_PIECES] = {
    100, 330, 320, 500, 900, 900, 
    100, 330, 320, 500, 900, 900
};

const int pieceSquareTables[NUM_PIECES][NUM_SQUARES] = {
    // WPAWN
    {
         0,   0,   0,   0,   0,   0,   0,   0,   
         5,  10,  10, -20, -20,  10,  10,   5,   
         5,  -5, -10,   0,   0, -10,  -5,   5,   
         0,   0,   0,  20,  20,   0,   0,   0,   
         5,   5,  10,  25,  25,  10,   5,   5,
         10,  10,  20,  30,  30,  20,  10,  10,
         50,  50,  50,  50,  50,  50,  50,  50,
         0,   0,   0,   0,   0,   0,   0,   0
    },
    // WBISHOP
    {
       -20, -10, -10, -10, -10, -10, -10, -20,
       -10,   5,   0,   0,   0,   0,   5, -10,
       -10,  10,  10,  10,  10,  10,  10, -10,
       -10,   0,  10,  10,  10,  10,   0, -10,
       -10,   5,   5,  10,  10,   5,   5, -10,
       -10,   0,   5,  10,  10,   5,   0, -10,
       -10,   0,   0,   0,   0,   0,   0, -10,
       -20, -10, -10, -10, -10, -10, -10, -20
    },
    // WKNIGHT
    {
       -50, -40, -30, -30, -30, -30, -40, -50,
       -40, -20,   0,   5,   5,   0, -20, -40,
       -30,   5,  10,  15,  15,  10,   5, -30,
       -30,   0,  15,  20,  20,  15,   0, -30,
       -30,   5,  15,  20,  20,  15,   5, -30,
       -30,   0,  10,  15,  15,  10,   0, -30,
       -40, -20,   0,   0,   0,   0, -20, -40,
       -50, -40, -30, -30, -30, -30, -40, -50
    },
    // WROOK
    {
         0,   0,   0,   5,   5,   0,   0,   0,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
         0,   0,   0,   0,   0,   0,   0,   0,
         5,  10,  10,  10,  10,  10,  10,   5
    },
    // WQUEEN
    {
       -20, -10, -10,  -5,  -5, -10, -10, -20,
       -10,   0,   0,   0,   0,   0,   0, -10,
       -10,   0,   5,   5,   5,   5,   0, -10,
        -5,   0,   5,   5,   5,   5,   0,  -5,
        -5,   0,   5,   5,   5,   5,   0,  -5,
       -10,   5,   5,   5,   5,   5,   5, -10,
       -10,   0,   0,   0,   0,   0,   0, -10,
       -20, -10, -10,  -5,  -5, -10, -10, -20
    },
    // WKING
    {
        20,  30,  10,   0,   0,  10,  30,  20,
        20,  20,   0,   0,   0,   0,  20,  20,
       -10, -20, -20, -20, -20, -20, -20, -10,
       -20, -30, -30, -40, -40, -30, -30, -20,
       -30, -40, -40, -50, -50, -40, -40, -30,
       -30, -40, -40, -50, -50, -40, -40, -30,
       -30, -40, -40, -50, -50, -40, -40, -30,
       -30, -40, -40, -50, -50, -40, -40, -30,
    },
    // BPAWN (negative mirror of WPAWN)
    {
         0,   0,   0,   0,   0,   0,   0,   0,
       -50, -50, -50, -50, -50, -50, -50, -50,
       -10, -10, -20, -30, -30, -20, -10, -10,
        -5,  -5, -10, -25, -25, -10,  -5,  -5,
         0,   0,   0, -20, -20,   0,   0,   0,
        -5,   5,  10,   0,   0,  10,   5,  -5,
        -5, -10, -10,  20,  20, -10, -10,  -5,
         0,   0,   0,   0,   0,   0,   0,   0,
    },
    // BBISHOP (negative mirror of WBISHOP)
    {
        20, -10, -10, -10, -10, -10, -10,  20,
        10,   0,   0,   0,   0,   0,   0,  10,
        10,   0,  -5, -10, -10,  -5,   0,  10,
        10,  -5,  -5, -10, -10,  -5,  -5,  10,
        10,   0, -10, -10, -10, -10,   0,  10,
        10, -10, -10, -10, -10, -10, -10,  10,
        10,  -5,   0,   0,   0,   0,  -5,  10,
        20,  10,  10,  10,  10,  10,  10,  20
    },
    // BKNIGHT (negative mirror of WKNIGHT)
    {
        50,  40,  30,  30,  30,  30,  40,  50,
        40,  20,   0,   0,   0,   0,  20,  40,
        30,   0, -10, -15, -15, -10,  0,  30,
        30,  -5, -15, -20, -20, -15,  -5,  30,
        30,   0, -15, -20, -20, -15,   0,  30,
        30,  -5, -10, -15, -15, -10,  -5,  30,
        40,  20,   0,  -5,  -5,   0,  20,  40,
        50,  40,  30,  30,  30,  30,  40,  50
    },
    // BROOK (negative mirror of WROOK)
    {
       -5, -10, -10, -10, -10, -10, -10,  -5,
        0,   0,   0,   0,   0,   0,   0,   0,
        5,   0,   0,   0,   0,   0,   0,   5,
        5,   0,   0,   0,   0,   0,   0,   5,
        5,   0,   0,   0,   0,   0,   0,   5,
        5,   0,   0,   0,   0,   0,   0,   5,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,  -5,  -5,   0,   0,   0
    },
    // BQUEEN (negative mirror of WQUEEN)
    {
       20,  10,  10,   5,   5,  10,  10,  20,
       10,   0,   0,   0,   0,   0,   0,  10,
       10,  -5,  -5,  -5,  -5,  -5,  -5,  10,
        5,   0,  -5,  -5,  -5,  -5,   0,   5,
        5,   0,  -5,  -5,  -5,  -5,   0,   5,
       10,   0,  -5,  -5,  -5,  -5,   0,   5,
       10,   0,   0,   0,   0,   0,   0,   0,
       20,  10,  10,   5,   5,  10,  10,  20
    },
    // BKING (negative mirror of WKING)
    {
        30,  40,  40,  50,  50,  40,  40,  30,
        30,  40,  40,  50,  50,  40,  40,  30,
        30,  40,  40,  50,  50,  40,  40,  30,
        30,  40,  40,  50,  50,  40,  40,  30,
        20,  30,  30,  40,  40,  30,  30,  20,
        10,  20,  20,  20,  20,  20,  20,  10,
       -20, -20,   0,   0,   0,   0, -20, -20,
       -20, -30, -10,   0,   0, -10, -30, -20
    }
};