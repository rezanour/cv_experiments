#pragma once

class Graphics
{
public:
    Graphics() = default;
    ~Graphics();

    // Noncopyable
    Graphics(Graphics const &) = delete;
    Graphics &operator= (Graphics const&) = delete;

    bool Initialize(void * const window_handle);
    void UpdateSource(uint8_t const *pixel_data, uint32_t const width, uint32_t const height);
    bool Refresh(bool const wait_for_vsync);

private:
    bool CreateDeviceResources(HWND const hwnd);
    bool CreateShader();
    bool CreateQuadAndTexture();

private:
    ComPtr<ID3D11Device>           device_;
    ComPtr<ID3D11DeviceContext>    context_;
    ComPtr<ID3D11RenderTargetView> rtv_;
    ComPtr<IDXGISwapChain>         swap_chain_;

    // simple shader to render image
    ComPtr<ID3D11InputLayout>  input_layout_;
    ComPtr<ID3D11VertexShader> vertex_shader_;
    ComPtr<ID3D11PixelShader>  pixel_shader_;
    ComPtr<ID3D11SamplerState> sampler_state_;

    // simple quad to render fullscreen
    ComPtr<ID3D11Buffer> quad_vb_;
    ComPtr<ID3D11Buffer> quad_ib_;

    // texture to upload images to for rendering
    ComPtr<ID3D11Texture2D>          src_texture_;
    ComPtr<ID3D11ShaderResourceView> src_srv_;
};
