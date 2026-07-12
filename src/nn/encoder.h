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

// Compact side-to-move-oriented position for training data files. The float
// planes above are derivable from this: bb[i] bit s -> plane i square s,
// flags bits 0-3 -> planes 12-15, epColumn -> plane 16, flags bit 4 -> plane 17.
#pragma pack(push, 1)
struct CompactPosition {
    uint64_t bb[12];   // our P,B,N,R,Q,K then their P,B,N,R,Q,K
    uint8_t flags;     // bits 0-3: castling our K, our Q, their K, their Q; bit 4: repetition
    uint8_t epColumn;  // 0-7, 255 = none
};
#pragma pack(pop)

void encodeCompact(const Board& board, CompactPosition& out);

}
