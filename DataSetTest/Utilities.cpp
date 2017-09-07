#include "Precomp.h"
#include "Utilities.h"

static float const Pi = 3.141592654f;

void GenerateGaussian(float const sigma, uint32_t const size, GaussianKernel *out_kernel)
{
    out_kernel->sigma = sigma;
    out_kernel->values.clear();

    float const center = static_cast<float>(size - 1) / 2;
    float sum = 0.0f;
    for (int32_t i = 0; i < static_cast<int32_t>(size); ++i)
    {
        float const x = static_cast<float>(i) - center;
        out_kernel->values.push_back(expf(-(x*x) / (2 * (sigma*sigma))) / (sqrtf(2 * Pi) * sigma));
        sum += out_kernel->values.back();
    }
    out_kernel->scale = 1.0f / sum;
}

void SmoothImage(uint8_t const *input, int32_t const width, int32_t const height, GaussianKernel const &kernel, uint8_t *scratch, uint8_t *output)
{
    // horizontal pass
    for (int32_t y = 0; y < height; ++y)
    {
        for (int32_t x = 0; x < width; ++x)
        {
            scratch[y * width + x] = static_cast<uint8_t>(Convolve(input, width, x, y, kernel.values.data(), 1, static_cast<int32_t>(kernel.values.size())) * kernel.scale);
        }
    }

    // vertical pass
    for (int32_t y = 0; y < height; ++y)
    {
        for (int32_t x = 0; x < width; ++x)
        {
            output[y * width + x] = static_cast<uint8_t>(Convolve(scratch, width, x, y, kernel.values.data(), static_cast<int32_t>(kernel.values.size()), 1) * kernel.scale);
        }
    }
}

uint32_t Convolve(uint8_t const *input, int32_t const stride, int32_t const x, int32_t const y, float const *kernel, int32_t const kernel_rows, int32_t const kernel_columns)
{
    int32_t const half_kernel_rows = kernel_rows / 2;
    int32_t const half_kernel_cols = kernel_columns / 2;

    float accum = 0.0f;
    for (int32_t ky = 0; ky < kernel_rows; ++ky)
    {
        int32_t const iy = y - half_kernel_rows + ky;
        for (int32_t kx = 0; kx < kernel_columns; ++kx)
        {
            int32_t const ix = x - half_kernel_cols + kx;
            accum += kernel[ky * kernel_columns + kx] * input[iy * stride + ix];
        }
    }
    return static_cast<uint32_t>(accum);
}
