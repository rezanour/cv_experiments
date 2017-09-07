#include "Precomp.h"
#include "Graphics.h"

// shaders
#include "passthrough_vs.h"
#include "quad_ps.h"

Graphics::~Graphics()
{
}

bool Graphics::Initialize(void * const window_handle)
{
    return CreateDeviceResources(reinterpret_cast<HWND const>(window_handle))
        && CreateShader()
        && CreateQuadAndTexture();
}

void Graphics::UpdateSource(uint8_t const *pixel_data, uint32_t const width, uint32_t const height)
{
    D3D11_BOX box{};
    box.right  = std::min(240u, width);
    box.bottom = std::min(180u, height);
    box.back   = 1;
    context_->UpdateSubresource(src_texture_.Get(), 0, &box, pixel_data, sizeof(uint8_t) * width, sizeof(uint8_t) * width * height);
}

bool Graphics::Refresh(bool const wait_for_vsync)
{
    float const clear_color[]{ 0.f, 0.f, 1.f, 1.f };
    context_->ClearRenderTargetView(rtv_.Get(), clear_color);
    context_->DrawIndexed(6, 0, 0);
    CHECKHR(swap_chain_->Present(wait_for_vsync ? 1 : 0, 0));
    return true;
}

bool Graphics::CreateDeviceResources(HWND const hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);

    uint32_t flags = 0;
#if _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL const feature_level = D3D_FEATURE_LEVEL_11_0;

    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount       = 2;
    scd.BufferDesc.Width  = rc.right - rc.left;
    scd.BufferDesc.Height = rc.bottom - rc.top;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow      = hwnd;
    scd.SampleDesc.Count  = 1;
    scd.Windowed          = TRUE;

    CHECKHR(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, &feature_level, 1,
        D3D11_SDK_VERSION, &scd, swap_chain_.ReleaseAndGetAddressOf(),
        device_.ReleaseAndGetAddressOf(), nullptr, context_.ReleaseAndGetAddressOf()));

    ComPtr<ID3D11Texture2D> texture;
    CHECKHR(swap_chain_->GetBuffer(0, IID_PPV_ARGS(&texture)));
    CHECKHR(device_->CreateRenderTargetView(texture.Get(), nullptr, rtv_.ReleaseAndGetAddressOf()));

    D3D11_VIEWPORT vp{};
    vp.Width    = static_cast<float>(scd.BufferDesc.Width);
    vp.Height   = static_cast<float>(scd.BufferDesc.Height);
    vp.MaxDepth = 1.0f;

    context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
    context_->RSSetViewports(1, &vp);

    return true;
}

bool Graphics::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC elems[2]{};
    elems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
    elems[0].SemanticName = "POSITION";
    elems[1].AlignedByteOffset = sizeof(XMFLOAT2);
    elems[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    elems[1].SemanticName = "TEXCOORD";
    CHECKHR(device_->CreateInputLayout(elems, _countof(elems), passthrough_vs, sizeof(passthrough_vs), input_layout_.ReleaseAndGetAddressOf()));
    CHECKHR(device_->CreateVertexShader(passthrough_vs, sizeof(passthrough_vs), nullptr, vertex_shader_.ReleaseAndGetAddressOf()));
    CHECKHR(device_->CreatePixelShader(quad_ps, sizeof(quad_ps), nullptr, pixel_shader_.ReleaseAndGetAddressOf()));

    D3D11_SAMPLER_DESC sd{};
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    CHECKHR(device_->CreateSamplerState(&sd, sampler_state_.ReleaseAndGetAddressOf()));

    context_->IASetInputLayout(input_layout_.Get());
    context_->VSSetShader(vertex_shader_.Get(), nullptr, 0);
    context_->PSSetShader(pixel_shader_.Get(), nullptr, 0);
    context_->PSSetSamplers(0, 1, sampler_state_.GetAddressOf());
    return true;
}

bool Graphics::CreateQuadAndTexture()
{
    struct Vertex
    {
        XMFLOAT2 Position;
        XMFLOAT2 TexCoord;
    };

    Vertex const vertices[] =
    {
        { XMFLOAT2(-1, -1), XMFLOAT2(0, 1) },
        { XMFLOAT2(-1,  1), XMFLOAT2(0, 0) },
        { XMFLOAT2( 1,  1), XMFLOAT2(1, 0) },
        { XMFLOAT2( 1, -1), XMFLOAT2(1, 1) },
    };

    uint32_t const indices[] =
    {
        0, 1, 2,
        0, 2, 3,
    };

    D3D11_BUFFER_DESC bd{};
    bd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth           = sizeof(vertices);
    bd.StructureByteStride = sizeof(vertices[0]);
    bd.Usage               = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem          = vertices;
    init.SysMemPitch      = sizeof(vertices);
    init.SysMemSlicePitch = sizeof(vertices);

    CHECKHR(device_->CreateBuffer(&bd, &init, quad_vb_.ReleaseAndGetAddressOf()));

    bd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth           = sizeof(indices);
    bd.StructureByteStride = sizeof(indices[0]);

    init.pSysMem          = indices;
    init.SysMemPitch      = sizeof(indices);
    init.SysMemSlicePitch = sizeof(indices);

    CHECKHR(device_->CreateBuffer(&bd, &init, quad_ib_.ReleaseAndGetAddressOf()));

    D3D11_TEXTURE2D_DESC td{};
    td.ArraySize        = 1;
    td.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
    td.Format           = DXGI_FORMAT_R8_UNORM;
    td.Width            = 240;
    td.Height           = 180;
    td.MipLevels        = 1;
    td.SampleDesc.Count = 1;
    td.Usage            = D3D11_USAGE_DEFAULT;
    CHECKHR(device_->CreateTexture2D(&td, nullptr, src_texture_.ReleaseAndGetAddressOf()));
    CHECKHR(device_->CreateShaderResourceView(src_texture_.Get(), nullptr, src_srv_.ReleaseAndGetAddressOf()));

    uint32_t const stride = sizeof(Vertex);
    uint32_t const offset = 0;
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->IASetVertexBuffers(0, 1, quad_vb_.GetAddressOf(), &stride, &offset);
    context_->IASetIndexBuffer(quad_ib_.Get(), DXGI_FORMAT_R32_UINT, 0);
    context_->PSSetShaderResources(0, 1, src_srv_.GetAddressOf());
    return true;
}
