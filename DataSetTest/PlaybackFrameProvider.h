#pragma once

#include "FrameProvider.h"

class PlaybackFrameProvider
    : private NonCopyable
    , public FrameProvider
{
public:
    PlaybackFrameProvider();
    ~PlaybackFrameProvider();

    bool Initialize(char const *data_path, bool const loop_playback);

    // FrameProvider
    virtual bool GetNextFrame(CameraFrame *out_frame) override;

private:
    struct ImageInfo
    {
        uint64_t     timestamp_us = 0;
        std::string  file_path;
    };

private:
    HRESULT const              hr_coinit_ = S_OK;
    ComPtr<IWICImagingFactory> factory_;
    std::vector<ImageInfo>     image_list_;
    size_t                     current_frame_ = 0;
    uint64_t                   start_timestamp_us_ = 0;
    bool                       loop_playback_ = false;
};
