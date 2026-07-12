#include "network.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>

#ifdef USE_ACCELERATE
#include <Accelerate/Accelerate.h>
#endif

namespace nn {

static constexpr float BN_EPS = 1e-5f;

static bool readFloats(std::ifstream& f, std::vector<float>& out, size_t n) {
    out.resize(n);
    f.read(reinterpret_cast<char*>(out.data()), n * sizeof(float));
    return (bool)f;
}

// Fold BN into conv: w *= gamma/sqrt(var+eps) per out channel, bias = beta - mean*scale
static bool readConvBn(std::ifstream& f, std::vector<float>& w, std::vector<float>& b,
                       int outC, size_t weightsPerOut) {
    if (!readFloats(f, w, (size_t)outC * weightsPerOut)) return false;
    std::vector<float> gamma, beta, mean, var;
    if (!readFloats(f, gamma, outC) || !readFloats(f, beta, outC) ||
        !readFloats(f, mean, outC) || !readFloats(f, var, outC)) return false;

    b.resize(outC);
    for (int oc = 0; oc < outC; oc++) {
        float scale = gamma[oc] / std::sqrt(var[oc] + BN_EPS);
        for (size_t i = 0; i < weightsPerOut; i++) {
            w[oc * weightsPerOut + i] *= scale;
        }
        b[oc] = beta[oc] - mean[oc] * scale;
    }
    return true;
}

bool Network::load(const std::string& path) {
    loaded = false;
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    int32_t magic = 0, version = 0, c32 = 0, b32 = 0;
    f.read(reinterpret_cast<char*>(&magic), 4);
    f.read(reinterpret_cast<char*>(&version), 4);
    f.read(reinterpret_cast<char*>(&c32), 4);
    f.read(reinterpret_cast<char*>(&b32), 4);
    if (!f || magic != 0x53534E4E || version != 1 || c32 <= 0 || c32 > 1024 || b32 <= 0 || b32 > 64) {
        return false;
    }
    C = c32;
    B = b32;

    if (!readConvBn(f, stemW, stemB, C, (size_t)PLANES * 9)) return false;

    blockW1.assign(B, {}); blockB1.assign(B, {});
    blockW2.assign(B, {}); blockB2.assign(B, {});
    for (int i = 0; i < B; i++) {
        if (!readConvBn(f, blockW1[i], blockB1[i], C, (size_t)C * 9)) return false;
        if (!readConvBn(f, blockW2[i], blockB2[i], C, (size_t)C * 9)) return false;
    }

    if (!readConvBn(f, polW, polB, 2, C)) return false;
    if (!readFloats(f, polFcW, (size_t)POLICY_SIZE * 128)) return false;
    if (!readFloats(f, polFcB, POLICY_SIZE)) return false;

    if (!readConvBn(f, valW, valB, 1, C)) return false;
    if (!readFloats(f, valFc1W, 256 * 64)) return false;
    if (!readFloats(f, valFc1B, 256)) return false;
    if (!readFloats(f, valFc2W, 256)) return false;
    if (!readFloats(f, valFc2B, 1)) return false;

    // must be exactly at EOF
    f.peek();
    if (!f.eof()) return false;

    buf1.resize((size_t)C * 64);
    buf2.resize((size_t)C * 64);
    buf3.resize((size_t)C * 64);
    colBuf.resize((size_t)(C > PLANES ? C : PLANES) * 9 * 64);
    loaded = true;
    return true;
}

#ifdef USE_ACCELERATE
// Unroll the 8x8 input into a [inC*9][64] matrix (zero padded) so the whole
// conv becomes one SGEMM: out[outC][64] = W[outC][inC*9] * col[inC*9][64]
static void im2col(const float* in, int inC, float* col) {
    for (int ic = 0; ic < inC; ic++) {
        const float* ip = in + ic * 64;
        for (int k = 0; k < 9; k++) {
            int dr = k / 3 - 1, dc = k % 3 - 1;
            float* row = col + ((size_t)ic * 9 + k) * 64;
            for (int r = 0; r < 8; r++) {
                int rr = r + dr;
                if (rr < 0 || rr > 7) {
                    std::memset(row + r * 8, 0, 8 * sizeof(float));
                    continue;
                }
                for (int c = 0; c < 8; c++) {
                    int cc = c + dc;
                    row[r * 8 + c] = (cc < 0 || cc > 7) ? 0.0f : ip[rr * 8 + cc];
                }
            }
        }
    }
}
#endif

static void conv3x3(const float* in, int inC, float* out, int outC,
                    const std::vector<float>& w, const std::vector<float>& b, bool relu,
                    float* col) {
#ifdef USE_ACCELERATE
    im2col(in, inC, col);
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, outC, 64, inC * 9,
                1.0f, w.data(), inC * 9, col, 64, 0.0f, out, 64);
    for (int oc = 0; oc < outC; oc++) {
        for (int cell = 0; cell < 64; cell++) {
            float s = out[oc * 64 + cell] + b[oc];
            out[oc * 64 + cell] = (relu && s < 0) ? 0 : s;
        }
    }
#else
    (void)col;
    for (int oc = 0; oc < outC; oc++) {
        const float* wp = &w[(size_t)oc * inC * 9];
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                float s = b[oc];
                for (int ic = 0; ic < inC; ic++) {
                    const float* ip = in + ic * 64;
                    const float* k = wp + ic * 9;
                    for (int dr = -1; dr <= 1; dr++) {
                        int rr = r + dr;
                        if (rr < 0 || rr > 7) continue;
                        for (int dc = -1; dc <= 1; dc++) {
                            int cc = c + dc;
                            if (cc < 0 || cc > 7) continue;
                            s += k[(dr + 1) * 3 + (dc + 1)] * ip[rr * 8 + cc];
                        }
                    }
                }
                out[oc * 64 + r * 8 + c] = (relu && s < 0) ? 0 : s;
            }
        }
    }
#endif
}

static void conv1x1(const float* in, int inC, float* out, int outC,
                    const std::vector<float>& w, const std::vector<float>& b) {
#ifdef USE_ACCELERATE
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, outC, 64, inC,
                1.0f, w.data(), inC, in, 64, 0.0f, out, 64);
    for (int oc = 0; oc < outC; oc++) {
        for (int cell = 0; cell < 64; cell++) {
            float s = out[oc * 64 + cell] + b[oc];
            out[oc * 64 + cell] = s > 0 ? s : 0;
        }
    }
#else
    for (int oc = 0; oc < outC; oc++) {
        for (int cell = 0; cell < 64; cell++) {
            float s = b[oc];
            for (int ic = 0; ic < inC; ic++) {
                s += w[(size_t)oc * inC + ic] * in[ic * 64 + cell];
            }
            out[oc * 64 + cell] = s > 0 ? s : 0;
        }
    }
#endif
}

float Network::evaluate(const float* planes, float* policyLogits) {
    float* a = buf1.data();
    float* t = buf2.data();
    float* u = buf3.data();
    float* col = colBuf.data();

    conv3x3(planes, PLANES, a, C, stemW, stemB, true, col);

    for (int i = 0; i < B; i++) {
        conv3x3(a, C, t, C, blockW1[i], blockB1[i], true, col);
        conv3x3(t, C, u, C, blockW2[i], blockB2[i], false, col);
        for (int j = 0; j < C * 64; j++) {
            float s = a[j] + u[j];
            a[j] = s > 0 ? s : 0;
        }
    }

    float p[128];
    conv1x1(a, C, p, 2, polW, polB);
    float v0[64];
    conv1x1(a, C, v0, 1, valW, valB);
    float h[256];

#ifdef USE_ACCELERATE
    std::memcpy(policyLogits, polFcB.data(), POLICY_SIZE * sizeof(float));
    cblas_sgemv(CblasRowMajor, CblasNoTrans, POLICY_SIZE, 128,
                1.0f, polFcW.data(), 128, p, 1, 1.0f, policyLogits, 1);

    std::memcpy(h, valFc1B.data(), 256 * sizeof(float));
    cblas_sgemv(CblasRowMajor, CblasNoTrans, 256, 64,
                1.0f, valFc1W.data(), 64, v0, 1, 1.0f, h, 1);
    for (int i = 0; i < 256; i++) {
        if (h[i] < 0) h[i] = 0;
    }
    float v = valFc2B[0] + cblas_sdot(256, valFc2W.data(), 1, h, 1);
#else
    for (int i = 0; i < POLICY_SIZE; i++) {
        float s = polFcB[i];
        const float* wp = &polFcW[(size_t)i * 128];
        for (int j = 0; j < 128; j++) s += wp[j] * p[j];
        policyLogits[i] = s;
    }

    for (int i = 0; i < 256; i++) {
        float s = valFc1B[i];
        const float* wp = &valFc1W[(size_t)i * 64];
        for (int j = 0; j < 64; j++) s += wp[j] * v0[j];
        h[i] = s > 0 ? s : 0;
    }
    float v = valFc2B[0];
    for (int j = 0; j < 256; j++) v += valFc2W[j] * h[j];
#endif

    return std::tanh(v);
}

}
