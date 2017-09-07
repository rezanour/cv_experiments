#include "Precomp.h"
#include "AppWindow.h"
#include "PlaybackFrameProvider.h"
#include "Graphics.h"
#include "FeatureDetector.h"
#include "Utilities.h"

struct Params
{
    char const *data_root = nullptr;
    LogLevel log_level = LogLevel::Verbose;
    bool log_to_console = true;
};

static void PrintUsage();
static void CommandLineParse(int32_t const argc, char const *argv[], Params *out_params);

int __cdecl main(int32_t const argc, char const *argv[])
{
    // Enable logging to console during command line parsing
    LogToConsole(true);

    Params params;
    CommandLineParse(argc, argv, &params);

    SetLogLevel(params.log_level);
    LogToConsole(params.log_to_console);

    // Check for necessary params
    if (!params.data_root)
    {
        LOGE("Required --root parameter not provided.");
        PrintUsage();
        return 0;
    }

    std::unique_ptr<AppWindow> window = std::make_unique<AppWindow>();
    if (!window->Initialize("DataSet Test", 1280, 960))
    {
        LOGF("Failed to initialize window");
    }

    std::unique_ptr<Graphics> graphics = std::make_unique<Graphics>();
    if (!graphics->Initialize(window->GetHandle()))
    {
        LOGF("Failed to initialize graphics");
    }

    window->Show(true);

    LOGD("Initializing playback frame provider with root [%s]", params.data_root)
    std::unique_ptr<PlaybackFrameProvider> frame_provider = std::make_unique<PlaybackFrameProvider>();
    if (!frame_provider->Initialize(params.data_root, true))
    {
        LOGF("Failed to initialize playback provider");
    }

    std::unique_ptr<FeatureDetector> detector = std::make_unique<FeatureDetector>();

    CameraFrame frame;
    std::vector<uint8_t> scratch;
    std::vector<uint8_t> smoothed;

    GaussianKernel smooth_kernel;
    GenerateGaussian(0.5f, 9, &smooth_kernel);

    window->Run([&]() 
    {
        if (!frame_provider->GetNextFrame(&frame))
        {
            LOGE("Failed to get next frame from provider");
            return false;
        }

        scratch.resize(frame.data.size());
        smoothed.resize(frame.data.size());

        SmoothImage(frame.data.data(), frame.width, frame.height, smooth_kernel, scratch.data(), smoothed.data());

        detector->Detect(frame.data.data(), smoothed.data(), frame.width, frame.height);
        graphics->UpdateSource(frame.data.data(), frame.width, frame.height);

        if (!graphics->Refresh(true))
        {
            LOGE("Failed to refresh graphics");
            return false;
        }

        return true;
    });

    frame_provider.reset();
    graphics.reset();
    window.reset();
    return 0;
}

void CommandLineParse(int32_t const argc, char const *argv[], Params *out_params)
{
    for (int32_t i = 1; i + 1 < argc; ++i)
    {
        if (0 == strcmp(argv[i], "--root"))
        {
            out_params->data_root = argv[i + 1];
        }
        else if (0 == strcmp(argv[i], "--loglevel"))
        {
            if (isalpha(argv[i + 1][0]))
            {
                if (0 == _stricmp(argv[i + 1], "FATAL"))
                {
                    out_params->log_level = LogLevel::Fatal;
                }
                else if (0 == _stricmp(argv[i + 1], "ERROR"))
                {
                    out_params->log_level = LogLevel::Error;
                }
                else if (0 == _stricmp(argv[i + 1], "WARNING"))
                {
                    out_params->log_level = LogLevel::Warning;
                }
                else if (0 == _stricmp(argv[i + 1], "DEBUG"))
                {
                    out_params->log_level = LogLevel::Debug;
                }
                else if (0 == _stricmp(argv[i + 1], "INFO"))
                {
                    out_params->log_level = LogLevel::Info;
                }
                else if (0 == _stricmp(argv[i + 1], "VERBOSE"))
                {
                    out_params->log_level = LogLevel::Verbose;
                }
                else
                {
                    LOGE("Invalid log level specified");
                }
            }
            else
            {
                int32_t const log_level = atoi(argv[i + 1]);
                if (log_level >= 0 && log_level < static_cast<int32_t>(LogLevel::MaxLogLevels))
                {
                    out_params->log_level = static_cast<LogLevel>(log_level);
                }
                else
                {
                    LOGE("Invalid log level specified.");
                }
            }
        }
        else if (0 == strcmp(argv[i], "--logconsole"))
        {
            if (isalpha(argv[i + 1][0]))
            {
                if (0 == _stricmp(argv[i + 1], "TRUE"))
                {
                    out_params->log_to_console = true;
                }
                else if (0 == _stricmp(argv[i + 1], "FALSE"))
                {
                    out_params->log_to_console = false;
                }
                else
                {
                    LOGE("Invalid log to console parameter specified");
                }
            }
            else
            {
                int32_t const log_to_console = atoi(argv[i + 1]);
                if (log_to_console >= 0 && log_to_console <= 1)
                {
                    out_params->log_to_console = !!log_to_console;
                }
                else
                {
                    LOGE("Invalid log to console parameter specified");
                }
            }
        }
    }
}

void PrintUsage()
{
    wprintf(
        L"USAGE:\n"
        L"  --root <path_to_data>       (REQUIRED) Path to source data for playback.\n"
        L"  --loglevel <level>          Set log filter level. Values are Fatal (0), Error (1),\n"
        L"                                  Warning (2), Debug (3), Info (4), and Verbose (5)\n"
        L"  --logconsole <true/false>   Enable logging to the console window.\n");
}
