#ifndef DIRECTX12_MANAGER_H
#define DIRECTX12_MANAGER_H

#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include <Windows.h>

HWND createWindow(const char* windowTitle, int width, int height) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, "DirectX12App", nullptr };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, windowTitle, WS_OVERLAPPEDWINDOW, 100, 100, width, height, nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    return hwnd;
}


class DirectX12Manager {
public:
    static DirectX12Manager& getInstance();

    void initialize(); // Initializes DX12 resources
    void createSwapChain(HWND hwnd); // Creates swap chain for rendering
    void renderFrame(); // Renders a frame (called by game thread or imgui thread)
    void cleanup(); // Cleanup resources

    // DirectX12 Resources
    ID3D12Device* device;
    ID3D12CommandQueue* commandQueue;
    ID3D12Fence* fence;

    
    Microsoft::WRL::ComPtr<IDXGISwapChain> gameSwapChain;
	HWND gameWindow;
    Microsoft::WRL::ComPtr<IDXGISwapChain> imguiSwapChain;
	HWND debugWindow;

private:
    DirectX12Manager() {}
    ~DirectX12Manager() {}

    // Helper method to create command lists, allocators, etc.
    void initializeResources();
};

#endif // DIRECTX12_MANAGER_H
