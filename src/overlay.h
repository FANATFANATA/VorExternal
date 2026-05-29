#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>

enum class OverlayStatus
{
    Success,
    WindowCreationFailed,
    DeviceCreationFailed,
    DwmExtensionFailed
};

class Overlay
{
public:
    Overlay() noexcept = default;
    ~Overlay() noexcept;

    Overlay(const Overlay &) = delete;
    Overlay &operator=(const Overlay &) = delete;
    Overlay(Overlay &&) noexcept = default;
    Overlay &operator=(Overlay &&) noexcept = default;

    [[nodiscard]] OverlayStatus initialize() noexcept;
    [[nodiscard]] bool handle_messages() noexcept;

    void begin_frame() noexcept;
    void end_frame() noexcept;

    void toggle_click_through(bool click_through) noexcept;

    [[nodiscard]] HWND get_window_handle() const noexcept;

private:
    HWND window_handle_{nullptr};
    WNDCLASSEXW window_class_{};

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_;

    static LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) noexcept;

    [[nodiscard]] bool create_overlay_window() noexcept;
    [[nodiscard]] bool initialize_directx() noexcept;
    void create_render_target() noexcept;
    void cleanup_render_target() noexcept;
};
