#pragma once

struct HarrisFeature
{
    int32_t x, y;
};

void HarrisDetect(uint8_t const *image, int32_t const width, int32_t const height, std::vector<HarrisFeature> *out_features);
