#pragma once

struct CameraFrame
{
    uint64_t timestamp_us;
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> data;
};

class FrameProvider
{
public:
    virtual ~FrameProvider() = default;

    virtual bool GetNextFrame(CameraFrame *out_frame) = 0;

protected:
    FrameProvider() = default;
};
