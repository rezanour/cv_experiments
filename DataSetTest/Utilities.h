#pragma once

uint32_t Convolve(uint8_t const *input, int32_t const stride, int32_t const x, int32_t const y, float const *kernel, int32_t const kernel_rows, int32_t const kernel_columns);

struct GaussianKernel
{
    float              sigma = 0.0f;
    float              scale = 1.0f;
    std::vector<float> values;
};

void GenerateGaussian(float const sigma, uint32_t const size, GaussianKernel *out_kernel);
void SmoothImage(uint8_t const *input, int32_t const width, int32_t const height, GaussianKernel const &kernel, uint8_t *scratch, uint8_t *output);

