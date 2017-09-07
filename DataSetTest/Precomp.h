#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wincodec.h>
#include <wrl.h>

#include <inttypes.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <nmmintrin.h>

#include <algorithm>
#include <chrono>
#include <istream>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "NonCopyable.h"
#include "Logging.h"

// Yes, I'm lazy and I know this is bad...
using Microsoft::WRL::ComPtr;
using namespace DirectX;

#define CHECKHR(x) \
{                                   \
    HRESULT hr##__LINE__ = (x);     \
    if (FAILED(hr##__LINE__))       \
    {                               \
        assert(false);              \
        return false;               \
    }                               \
}
