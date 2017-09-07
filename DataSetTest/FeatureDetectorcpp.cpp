#include "Precomp.h"
#include "FeatureDetector.h"
#include "Utilities.h"
#include "HarrisCorners.h"

struct FAST_feature
{
    int x, y;
    int score;
    uint64_t descriptor[2];
    uint32_t frame_count = 0;
};

static bool ComputeDescriptor(uint8_t const *source, int32_t const width, int32_t const height, int32_t const x, int32_t const y, uint64_t *inout_descriptor)
{
    static int32_t x_offsets[128]{};
    static int32_t y_offsets[128]{};
    static int32_t x_offsets2[128]{};
    static int32_t y_offsets2[128]{};
    static bool    offsets_initialized = false;

    if (!offsets_initialized)
    {
        srand(static_cast<int>(time(nullptr)));
        for (int32_t i = 0; i < 128; ++i)
        {
            x_offsets[i] = rand() % 16 - 8;
            y_offsets[i] = rand() % 16 - 8;
            x_offsets2[i] = rand() % 16 - 8;
            y_offsets2[i] = rand() % 16 - 8;
        }
        offsets_initialized = true;
    }

    if (x < 8 || x >= width - 8 || y < 8 || y >= height - 8)
    {
        return false;
    }

    inout_descriptor[0] = 0;
    inout_descriptor[1] = 0;
    for (int32_t i = 0; i < 128; ++i)
    {
        int32_t index = i / 64;
        int32_t bit   = i % 64;

        uint8_t val1 = source[(y + y_offsets[i]) * width + (x + x_offsets[i])];
        uint8_t val2 = source[(y + y_offsets2[i]) * width + (x + x_offsets2[i])];
        uint64_t bit_value = (val1 < val2) ? 1 : 0;
        inout_descriptor[index] |= (bit_value << bit);
    }

    return true;
}

//
// FAST (Features from Accelerated Segment Test) feature detector
//
// source       - input image as 8-bit luminance
// width        - number of columns in the image
// height       - number of rows in the image
// pixel_pitch  - distance between 2 consecutive rows, in pixels
// segment_size - length of segment before considering a pixel as a feature. Typical sizes are 9 & 12 (empirically, 9 performs better than 12)
// threshold    - how much brigher or darker than current pixel the segment pixels can be and still count
// max_features - maximum number of features to detect. once reached, the function will return even if the image has not been fully processed
// out_features - buffer to hold the detected features. Must be at least max_features in size
//
// returns: number of features actually detected (and stored in out_features)
//
static int FAST(uint8_t const* source, uint8_t const *smoothed, int width, int height, int pixel_pitch, uint8_t segment_size, uint8_t threshold, int max_features, FAST_feature* out_features)
{
    // x,y offsets to each of the pixels around the circle
    static int const x_offsets[] =
    {
        0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1,
    };
    static int const y_offsets[] =
    {
        -3, -3, -2, -1, 0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3
    };

    static const int num_offsets = _countof(x_offsets);

    static_assert(num_offsets == _countof(y_offsets), "x_offsets & y_offsets don't match in size");
    static_assert(num_offsets == 16, "expected 16 pixel circle");

    int num_features = 0;
    // NOTE: for simplicity, we start 3 pixels in each from all edges.
    // This allows us to always use the full circle, and not have to handle corner cases
    for (int y = 3; y < height - 3; ++y)
    {
        for (int x = 3; x < width - 3; ++x)
        {
            // Luminance of the candidate pixel
            uint8_t Ip = source[y * pixel_pitch + x];

            // Track consecutive dark & light pixels & scores
            // at the same time through a single loop
            int last_non_bright = -1;
            int last_non_dark = -1;
            int bright_score = 0;
            int dark_score = 0;

            int match_score = 0;

            for (int i = 0; i < num_offsets; ++i)
            {
                uint8_t I = source[(y + y_offsets[i]) * pixel_pitch + (x + x_offsets[i])];

                if (I <= Ip + threshold)
                {
                    last_non_bright = i;
                    bright_score = 0;
                } else
                {
                    bright_score += abs((int)I - (int)Ip) - threshold;
                }

                if (I >= Ip - threshold)
                {
                    last_non_dark = i;
                    dark_score = 0;
                } else
                {
                    dark_score += abs((int)Ip - (int)I) - threshold;
                }

                if (i - (last_non_bright + 1) == segment_size)
                {
                    match_score = bright_score;
                    break;
                }
                if (i - (last_non_dark + 1) == segment_size)
                {
                    match_score = dark_score;
                    break;
                }
            }

            if (match_score > 0)
            {
                out_features[num_features].x = x;
                out_features[num_features].y = y;
                out_features[num_features].score = match_score;
                out_features[num_features].frame_count = 0;
                if (ComputeDescriptor(smoothed, width, height, x, y, out_features[num_features].descriptor))
                {
                    ++num_features;
                    if (num_features == max_features)
                    {
                        return num_features;
                    }
                }
            }
        }
    }
    return num_features;
}

static uint32_t HammingDistance(uint64_t const descriptor[2], uint64_t const descriptor2[2])
{
    uint64_t dist1 = descriptor[0] ^ descriptor2[0];
    uint64_t dist2 = descriptor[1] ^ descriptor2[1];
    return static_cast<uint32_t>(_mm_popcnt_u64(dist1) + _mm_popcnt_u64(dist2));
}

bool FeatureDetector::Detect(uint8_t *pixels, uint8_t const *smoothed, uint32_t const width, uint32_t const height)
{
    UNREFERENCED_PARAMETER(smoothed);

    std::vector<HarrisFeature> features;
    HarrisDetect(smoothed, width, height, &features);
    for (auto const &feature : features)
    {
        pixels[feature.y * width + feature.x] = 0xFF;
    }
#if 0
    static FAST_feature prev_features[400]{};
    static int prev_num_features = 0;
    FAST_feature features[400];

    int num_features = FAST(pixels, smoothed, width, height, width, 9, 20, 100, features);
    for (int i = 0; i < num_features; ++i)
    {
        for (int j = 0; j < prev_num_features; ++j)
        {
            if (HammingDistance(features[i].descriptor, prev_features[j].descriptor) < 5)
            {
                features[i].frame_count = prev_features[j].frame_count + 1;
                if (features[i].frame_count > 5)
                {
                    pixels[features[i].y * width + features[i].x] = 0xFF;
                }
                break;
            }
        }
    }
    memcpy(prev_features, features, num_features * sizeof(FAST_feature));
    prev_num_features = num_features;
#endif
#if 0

    uint32_t num_features = 0;
    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            if (32 > pixels[y * width + x])
            {
                pixels[y * width + x] = 0xFF;
                Feature f;
                f.x = x;
                f.y = y;
                f.desc = 128;
                inout_features[num_features++] = f;
                if (num_features >= max_features)
                {
                    *out_num_features = num_features;
                    return true;
                }
            }
        }
    }
    *out_num_features = num_features;
#endif
    return true;
}
