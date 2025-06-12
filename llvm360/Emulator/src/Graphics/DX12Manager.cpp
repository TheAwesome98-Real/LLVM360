#include "DX12Manager.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <stdexcept>

DirectX12Manager& DirectX12Manager::getInstance() {
    static DirectX12Manager instance;
    return instance;
}

void DirectX12Manager::initialize() {
    // Create Device, CommandQueue, SwapChain, etc.
    // Setup DX12 device
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)))) {
        throw std::runtime_error("Failed to create DX12 device.");
    }

    // Create Command Queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)))) {
        throw std::runtime_error("Failed to create command queue.");
    }

    // Initialize resources
    initializeResources();
}

void DirectX12Manager::createSwapChain(HWND hwnd) {
    // Create swap chain for game window
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = 800;
    swapChainDesc.BufferDesc.Height = 600;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    //factory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, &gameSwapChain);

    // Create another swap chain for ImGui window if necessary
    // (similar setup)
}

void DirectX12Manager::renderFrame() {
   }

void DirectX12Manager::initializeResources() {
}

void DirectX12Manager::cleanup() {
}
