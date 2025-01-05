#include "evalConstants.h"

// King eval should never be used
const int materialEvaluations[NUM_PIECES] = {
    100, 330, 320, 500, 900, 20000, 
    -100, -330, -320, -500, -900, -20000
};

// Absolute value version of material evaluations. King weighted heavily to move back illegal moves
const int moveScoreMaterialEvaluations[NUM_PIECES] = {
    100, 330, 320, 500, 900, 20000, 
    100, 330, 320, 500, 900, 20000
};