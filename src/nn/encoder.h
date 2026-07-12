#pragma once

#include "../chess/board/board.h"
#include "../chess/board/move.h"

namespace nn {

constexpr int PLANES = 19;
constexpr int POLICY_SIZE = 4096;

// Planes (8x8 each, oriented so the side to move plays "up"; black positions
// are rank-mirrored via sq ^ 56 and colors swapped):
//   0-5   our P, B, N, R, Q, K
//   6-11  their P, B, N, R, Q, K
//   12-15 castling rights: our K, our Q, their K, their Q (whole plane)
//   16    en passant column
//   17    position occurred before (repetition, whole plane)
//   18    all ones
void encodeBoard(const Board& board, float* planes);

// from*64 + to in the same orientation; promotions share the from-to index
int policyIndex(const Move& move, bool whiteToMove);

}
