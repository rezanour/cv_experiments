#include "Precomp.h"
#include "PlaybackFrameProvider.h"

PlaybackFrameProvider::PlaybackFrameProvider()
    : hr_coinit_(CoInitialize(nullptr))
{
}

PlaybackFrameProvider::~PlaybackFrameProvider()
{
    factory_ = nullptr;
    if (SUCCEEDED(hr_coinit_))
    {
        CoUninitialize();
    }
}

bool PlaybackFrameProvider::Initialize(char const *data_path, bool const loop_playback)
{
    loop_playback_ = loop_playback;
    current_frame_ = 0;
    start_timestamp_us_ = 0;

    CHECKHR(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory_)));

    std::string root(data_path);
    if ('\\' != root[root.size() - 1])
    {
        root += "\\";
    }

    std::string calib_file_path = root + "calib.txt";
    std::ifstream calib_file(calib_file_path, std::ios::in);

    // TODO: Read in calibration

    std::string images_file_path = root + "images.txt";
    std::ifstream images_file(images_file_path, std::ios::in);

    ImageInfo    image;
    double       timestamp = 0;
    std::string  image_path;

    while (!images_file.eof())
    {
        images_file >> timestamp >> image_path;
        if (image_path.empty())
        {
            continue;
        }

        // Convert from seconds to microseconds
        image.timestamp_us = static_cast<uint64_t>(timestamp * 1000 * 1000);
        image.file_path = root + image_path;

        image_list_.push_back(image);
    }

    return true;
}

bool PlaybackFrameProvider::GetNextFrame(CameraFrame *out_frame)
{
    ImageInfo &image = image_list_[current_frame_];
    uint64_t const now_us = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()) / 1000;

    // If first frame, start the video stream
    if (0 == start_timestamp_us_)
    {
        start_timestamp_us_ = now_us;
    }

    // Advance the frames until we're at the right place in the stream
    while (now_us >= start_timestamp_us_ + image.timestamp_us)
    {
        if (current_frame_ + 1 < image_list_.size())
        {
            ++current_frame_;
            image = image_list_[current_frame_];
            continue;
        }
        else if (loop_playback_)
        {
            current_frame_ = 0;
            start_timestamp_us_ = now_us;
            image = image_list_[current_frame_];
            continue;
        }
        else
        {
            // stay on last frame
            break;
        }
    }

    out_frame->timestamp_us = start_timestamp_us_ + image.timestamp_us;

    ComPtr<IWICBitmapDecoder> decoder;
    ComPtr<IWICBitmapFrameDecode> frame;
    std::wstring wide_path(image.file_path.length(), ' ');
    std::copy(image.file_path.begin(), image.file_path.end(), wide_path.begin());
    CHECKHR(factory_->CreateDecoderFromFilename(wide_path.c_str(), nullptr, GENERIC_READ, WICDecodeOptions::WICDecodeMetadataCacheOnLoad, &decoder));
    CHECKHR(decoder->GetFrame(0, &frame));

    CHECKHR(frame->GetSize(&out_frame->width, &out_frame->height));
    out_frame->data.resize(out_frame->width * out_frame->height);

    CHECKHR(frame->CopyPixels(nullptr, out_frame->width * sizeof(uint8_t),
        out_frame->width * out_frame->height * sizeof(uint8_t), reinterpret_cast<BYTE *>(out_frame->data.data())));
    return true;
}
