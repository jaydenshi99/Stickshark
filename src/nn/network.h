#pragma once

#include <string>
#include <vector>

#include "encoder.h"

namespace nn {

// Weight file format (all little-endian; float32 tensors in PyTorch memory
// order, i.e. conv [out][in][kh][kw], linear [out][in]):
//   int32 magic = 0x53534E4E ("SSNN"), int32 version = 1
//   int32 channels C, int32 blocks B
//   stem:      conv [C][PLANES][3][3], bn gamma/beta/mean/var [C] each
//   block x B: conv1 [C][C][3][3], bn1 (4x[C]), conv2 [C][C][3][3], bn2 (4x[C])
//   policy:    conv [2][C][1][1], bn (4x[2]), fc [POLICY_SIZE][128] + bias [POLICY_SIZE]
//   value:     conv [1][C][1][1], bn (4x[1]), fc1 [256][64] + bias [256],
//              fc2 [1][256] + bias [1]
// BatchNorm (eps 1e-5) is folded into the preceding conv at load time.
class Network {
    public:
    bool load(const std::string& path);
    bool isLoaded() const { return loaded; }
    int channels() const { return C; }
    int blocks() const { return B; }

    // planes: PLANES*64 in. policyLogits: POLICY_SIZE out (unmasked).
    // Returns the value in [-1, 1] for the side to move.
    float evaluate(const float* planes, float* policyLogits);

    private:
    bool loaded = false;
    int C = 0, B = 0;

    std::vector<float> stemW, stemB;
    std::vector<std::vector<float>> blockW1, blockB1, blockW2, blockB2;
    std::vector<float> polW, polB, polFcW, polFcB;
    std::vector<float> valW, valB, valFc1W, valFc1B, valFc2W, valFc2B;

    std::vector<float> buf1, buf2, buf3;   // scratch, sized C*64
    std::vector<float> colBuf;             // im2col scratch for the BLAS path
};

}
