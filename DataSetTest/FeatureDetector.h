#pragma once

class FeatureDetector
{
public:
    struct Feature
    {
        uint32_t x;
        uint32_t y;
        uint32_t desc;
    };

public:
    FeatureDetector() = default;
    ~FeatureDetector() = default;

    // Noncopyable
    FeatureDetector(FeatureDetector const &) = delete;
    FeatureDetector &operator= (FeatureDetector const &) = delete;

    bool Detect(uint8_t *pixels, uint8_t const *smoothed, uint32_t const width, uint32_t const height);
private:
};
