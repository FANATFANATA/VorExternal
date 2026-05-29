#include "overlay.h"
#include <dwmapi.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Overlay::~Overlay() noexcept
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cleanup_render_target();
    if (window_handle_ != nullptr)
        DestroyWindow(window_handle_);
    if (window_class_.lpszClassName != nullptr)
        UnregisterClassW(window_class_.lpszClassName, window_class_.hInstance);
}

LRESULT CALLBACK Overlay::window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) noexcept
{
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
        return TRUE;
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, w_param, l_param);
    }
}

bool Overlay::create_overlay_window() noexcept
{
    window_class_.cbSize = sizeof(WNDCLASSEXW);
    window_class_.style = CS_HREDRAW | CS_VREDRAW;
    window_class_.lpfnWndProc = window_procedure;
    window_class_.hInstance = GetModuleHandleW(nullptr);
    window_class_.lpszClassName = L"VorOverlayClass";
    if (!RegisterClassExW(&window_class_))
        return false;
    const int screen_width = GetSystemMetrics(SM_CXSCREEN);
    const int screen_height = GetSystemMetrics(SM_CYSCREEN);
    window_handle_ = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        window_class_.lpszClassName,
        L"VorOverlay",
        WS_POPUP,
        0, 0, screen_width, screen_height,
        nullptr, nullptr, window_class_.hInstance, nullptr);
    if (window_handle_ == nullptr)
        return false;
    SetLayeredWindowAttributes(window_handle_, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margins = {-1, -1, -1, -1};
    if (FAILED(DwmExtendFrameIntoClientArea(window_handle_, &margins)))
        return false;
    ShowWindow(window_handle_, SW_SHOWDEFAULT);
    UpdateWindow(window_handle_);
    return true;
}

bool Overlay::initialize_directx() noexcept
{
    DXGI_SWAP_CHAIN_DESC swap_chain_description{};
    swap_chain_description.BufferCount = 2;
    swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_description.OutputWindow = window_handle_;
    swap_chain_description.SampleDesc.Count = 1;
    swap_chain_description.Windowed = TRUE;
    swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL feature_level{};
    const D3D_FEATURE_LEVEL feature_levels_requested[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    const HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_SINGLETHREADED,
        feature_levels_requested, 2, D3D11_SDK_VERSION, &swap_chain_description,
        &swap_chain_, &device_, &feature_level, &device_context_);
    if (FAILED(result))
        return false;
    create_render_target();
    return true;
}

void Overlay::create_render_target() noexcept
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (back_buffer)
        device_->CreateRenderTargetView(back_buffer.Get(), nullptr, &render_target_view_);
}

void Overlay::cleanup_render_target() noexcept
{
    render_target_view_.Reset();
}

OverlayStatus Overlay::initialize() noexcept
{
    if (!create_overlay_window())
        return OverlayStatus::WindowCreationFailed;
    if (!initialize_directx())
        return OverlayStatus::DeviceCreationFailed;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(window_handle_);
    ImGui_ImplDX11_Init(device_.Get(), device_context_.Get());
    return OverlayStatus::Success;
}

bool Overlay::handle_messages() noexcept
{
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
        if (message.message == WM_QUIT)
            return false;
    }
    return true;
}

void Overlay::begin_frame() noexcept
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Overlay::end_frame() noexcept
{
    ImGui::Render();
    constexpr float clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    device_context_->OMSetRenderTargets(1, render_target_view_.GetAddressOf(), nullptr);
    device_context_->ClearRenderTargetView(render_target_view_.Get(), clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swap_chain_->Present(1, 0);
}

void Overlay::toggle_click_through(bool click_through) noexcept
{
    LONG_PTR style = GetWindowLongPtrW(window_handle_, GWL_EXSTYLE);
    if (click_through)
        style |= WS_EX_TRANSPARENT;
    else
        style &= ~WS_EX_TRANSPARENT;
    SetWindowLongPtrW(window_handle_, GWL_EXSTYLE, style);
}
