#include "Precomp.h"
#include "HarrisCorners.h"
#include "Utilities.h"

static inline void ComputeM(uint8_t const *image, int32_t const stride, int32_t const x, int32_t const y, int32_t const window_half, int64_t out_m[2][2])
{
    static float sobel_operator_x[] =
    {
        1, 0, -1,
        2, 0, -2,
        1, 0, -1
    };
    static float sobel_operator_y[] =
    {
        1,  2,  1,
        0,  0,  0,
        -1, -2, -1
    };

    memset(out_m, 0, sizeof(int64_t) * 4);
    for (int32_t iy = y - window_half; iy <= y + window_half; ++iy)
    {
        for (int32_t ix = x - window_half; ix <= x + window_half; ++ix)
        {
            int64_t const Ix = static_cast<int64_t>(Convolve(image, stride, ix, iy, sobel_operator_x, 3, 3));
            int64_t const Iy = static_cast<int64_t>(Convolve(image, stride, ix, iy, sobel_operator_y, 3, 3));
            out_m[0][0] += (Ix * Ix);
            out_m[0][1] += Ix * Iy;
            out_m[1][0] += Ix * Iy;
            out_m[1][1] += (Iy * Iy);
        }
    }
}

void HarrisDetect(uint8_t const *image, int32_t const width, int32_t const height, std::vector<HarrisFeature> *out_features)
{
    static int32_t const  window_size = 3; // 3x3 with extents [-1, 1]

    int32_t const window_half = window_size / 2;
    float   const k           = 0.03f;

    int64_t M[2][2]{};
    int64_t detM = 0;
    int64_t traceM = 0;
    int64_t R = 0;

    out_features->clear();

    int64_t const threshold = static_cast<int64_t>(INT64_MAX) / 1000;

    // TODO: To avoid handling boundaries, we only search in the inner rectangle of the image
    // such that our entire evaluation window is within the image
    for (int32_t y = window_half + 1; y < height - window_half - 1; ++y)
    {
        for (int32_t x = window_half + 1; x < width - window_half - 1; ++x)
        {
            ComputeM(image, width, x, y, window_half, M);
            detM = (M[0][0] * M[1][1]) - (M[0][1] * M[1][0]);
            traceM = M[0][0] + M[1][1];
            R = static_cast<int64_t>(detM - k * (traceM * traceM));
            if (abs(R) > threshold)
            {
                HarrisFeature feature;
                feature.x = x;
                feature.y = y;
                out_features->push_back(feature);
            }
        }
    }
}
