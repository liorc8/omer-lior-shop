// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"  // Include the main ImGui header
#include "imgui_impl_win32.h"  // Include the ImGui implementation for Win32
#include "imgui_impl_dx11.h"  // Include the ImGui implementation for DirectX 11
#include <d3d11.h>  // Include the DirectX 11 header
#include <tchar.h>  // Include the header for working with TCHAR strings
#include <string>  // Include the standard string header
#include "GuiMain.h"  // Include the GuiMain header
#include <thread>  // Include the threading header
#include <chrono>  // Include the chrono header for timing
using namespace std::chrono_literals;  // Enable the use of duration literals
#include <vector>  // Include the standard vector header
#include <ranges>  // Include the ranges header for working with ranges

#define STB_IMAGE_IMPLEMENTATION  // Define this macro to implement the STB image library functions
#include "stb_image.h"  // Include the STB image header for image loading

// Data
static ID3D11Device* g_pd3dDevice = nullptr;  // Pointer to the Direct3D device
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;  // Pointer to the Direct3D device context
static IDXGISwapChain* g_pSwapChain = nullptr;  // Pointer to the DXGI swap chain
static bool                     g_SwapChainOccluded = false;  // Flag indicating if the swap chain is occluded
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;  // Variables to store the new width and height during resize
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;  // Pointer to the render target view

// Simple helper function to load an image into a DX11 texture with common settings
// Use like this:
// int my_image_width = 0;
// int my_image_height = 0;
// ID3D11ShaderResourceView* my_texture = NULL;
// bool ret = LoadTextureFromFile("../../MyImage01.jpg", &my_texture, &my_image_width, &my_image_height);
// IM_ASSERT(ret);
bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    // Load from disk into a raw RGBA buffer
    int image_width = 0;  // Variable to store the image width
    int image_height = 0;  // Variable to store the image height
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);  // Load the image from file using STB image
    if (image_data == NULL)  // Check if the image failed to load
        return false;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;  // Structure to describe the texture
    ZeroMemory(&desc, sizeof(desc));  // Zero out the memory for the texture description
    desc.Width = image_width;  // Set the texture width
    desc.Height = image_height;  // Set the texture height
    desc.MipLevels = 1;  // Set the number of mip levels
    desc.ArraySize = 1;  // Set the number of textures in the array
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Set the texture format
    desc.SampleDesc.Count = 1;  // Set the sample count
    desc.Usage = D3D11_USAGE_DEFAULT;  // Set the usage pattern
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;  // Set the bind flags
    desc.CPUAccessFlags = 0;  // No CPU access is needed

    ID3D11Texture2D* pTexture = NULL;  // Pointer to the texture
    D3D11_SUBRESOURCE_DATA subResource;  // Structure to describe the subresource data
    subResource.pSysMem = image_data;  // Pointer to the image data
    subResource.SysMemPitch = desc.Width * 4;  // The pitch of the memory (row size in bytes)
    subResource.SysMemSlicePitch = 0;  // No slice pitch is needed
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);  // Create the texture

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;  // Structure to describe the shader resource view
    ZeroMemory(&srvDesc, sizeof(srvDesc));  // Zero out the memory for the shader resource view description
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Set the format for the view
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;  // Set the view dimension
    srvDesc.Texture2D.MipLevels = desc.MipLevels;  // Set the number of mip levels in the view
    srvDesc.Texture2D.MostDetailedMip = 0;  // Set the most detailed mip level
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);  // Create the shader resource view
    pTexture->Release();  // Release the texture resource

    *out_width = image_width;  // Set the output width
    *out_height = image_height;  // Set the output height
    stbi_image_free(image_data);  // Free the image data

    return true;  // Return true to indicate success
}

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);  // Function to create the Direct3D device and swap chain
void CleanupDeviceD3D();  // Function to clean up the Direct3D device and swap chain
void CreateRenderTarget();  // Function to create the render target
void CleanupRenderTarget();  // Function to clean up the render target
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);  // Function to handle window messages

// RTL strings
// Function to determine the length of a UTF-8 character
size_t utf8_char_length(char c) {
    if ((c & 0x80) == 0x00) return 1;  // 1-byte character (ASCII)
    if ((c & 0xE0) == 0xC0) return 2;  // 2-byte character
    if ((c & 0xF0) == 0xE0) return 3;  // 3-byte character
    if ((c & 0xF8) == 0xF0) return 4;  // 4-byte character
    return 1;  // Fallback to 1-byte character if something goes wrong
}

// Function to reverse a UTF-8 string
std::string reverse_utf8(const std::string_view input) {
    std::vector<std::string_view> characters;  // Vector to store individual UTF-8 characters
    size_t i = 0;

    while (i < input.size()) {  // Iterate through the input string
        size_t char_len = utf8_char_length(input[i]);  // Determine the length of the current character
        characters.push_back(input.substr(i, char_len));  // Add the character to the vector
        i += char_len;  // Move to the next character
    }

    // Reverse the order of the characters and concatenate them using ranges and views
    auto reversed_view = characters | std::views::reverse | std::views::join;

    return std::string(reversed_view.begin(), reversed_view.end());  // Return the reversed string
}

// Main code

int GuiMain(drawcallback drawfunction, void* obj_ptr)
{
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    // Define and register the window class
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"omer & lior shop", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);
    // Create the application window with specified dimensions and position

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();  // Cleanup in case of failure
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);  // Unregister the window class
        return 1;  // Return an error code
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);  // Show the window
    ::UpdateWindow(hwnd);  // Update the window

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();  // Check the ImGui version
    ImGui::CreateContext();  // Create the ImGui context
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();  // Set the ImGui style to dark

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);  // Initialize the ImGui Win32 backend
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);  // Initialize the ImGui DirectX 11 backend

    // Load Fonts
    static const ImWchar ranges[] = { 0x0020, 0x00FF, 0x0590, 0x05FF, 0 };  // Define the font range (ASCII and Hebrew characters)
    ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\calibril.ttf", 18.0f, nullptr, &ranges[0]);  // Load the default font
    IM_ASSERT(defaultFont != nullptr);  // Assert if the font failed to load

    // Font for title
    ImFont* titleFont = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\calibril.ttf", 36.0f, nullptr, &ranges[0]);  // Load a larger font for titles
    IM_ASSERT(titleFont != nullptr);  // Assert if the font failed to load

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);  // Define the clear color

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))  // Peek for any messages in the queue
        {
            ::TranslateMessage(&msg);  // Translate the message
            ::DispatchMessage(&msg);  // Dispatch the message to the appropriate window procedure
            if (msg.message == WM_QUIT)  // Check if the quit message was received
                done = true;  // Set the done flag to true to exit the loop
        }
        if (done)  // If done is true, exit the loop
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);  // Sleep for 10 milliseconds
            continue;  // Skip the rest of the loop if the swap chain is occluded
        }
        g_SwapChainOccluded = false;  // Reset the swap chain occluded flag

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();  // Clean up the current render target
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);  // Resize the swap chain buffers
            g_ResizeWidth = g_ResizeHeight = 0;  // Reset the resize width and height
            CreateRenderTarget();  // Create a new render target
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();  // Start a new frame for the DirectX 11 backend
        ImGui_ImplWin32_NewFrame();  // Start a new frame for the Win32 backend
        ImGui::NewFrame();  // Start a new ImGui frame

        drawfunction(obj_ptr);  // Call the draw function

        // Rendering
        ImGui::Render();  // Render the ImGui frame
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };  // Compute the clear color with alpha
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);  // Set the render target
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);  // Clear the render target with the clear color
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());  // Render the ImGui draw data

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync enabled
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);  // Update the swap chain occluded flag if the present call returns occluded status
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();  // Shut down the DirectX 11 backend
    ImGui_ImplWin32_Shutdown();  // Shut down the Win32 backend
    ImGui::DestroyContext();  // Destroy the ImGui context

    CleanupDeviceD3D();  // Clean up the Direct3D device
    ::DestroyWindow(hwnd);  // Destroy the application window
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);  // Unregister the window class

    return 0;  // Return success
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;  // Swap chain description structure
    ZeroMemory(&sd, sizeof(sd));  // Zero out the memory for the swap chain description
    sd.BufferCount = 2;  // Double buffering
    sd.BufferDesc.Width = 0;  // Use the width of the window
    sd.BufferDesc.Height = 0;  // Use the height of the window
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Use a standard format
    sd.BufferDesc.RefreshRate.Numerator = 60;  // Set the refresh rate to 60 Hz
    sd.BufferDesc.RefreshRate.Denominator = 1;  // Set the refresh rate denominator
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  // Allow mode switching
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // Set buffer usage to render target output
    sd.OutputWindow = hWnd;  // Set the output window
    sd.SampleDesc.Count = 1;  // Set sample count to 1
    sd.SampleDesc.Quality = 0;  // Set sample quality to 0
    sd.Windowed = TRUE;  // Run in windowed mode
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;  // Discard the back buffer after presenting

    UINT createDeviceFlags = 0;  // Flags for device creation
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;  // Uncomment to enable debug layer
    D3D_FEATURE_LEVEL featureLevel;  // Variable to store the feature level
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };  // Array of feature levels to attempt
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    // Try to create the Direct3D device and swap chain with hardware acceleration
    if (res == DXGI_ERROR_UNSUPPORTED)  // If hardware is not available, try the WARP software driver
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)  // If device creation fails, return false
        return false;

    CreateRenderTarget();  // Create the render target
    return true;  // Return true to indicate success
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();  // Clean up the render target
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }  // Release the swap chain
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }  // Release the device context
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }  // Release the device
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;  // Pointer to the back buffer
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));  // Get the back buffer from the swap chain
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);  // Create the render target view from the back buffer
    pBackBuffer->Release();  // Release the back buffer
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }  // Release the render target view
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))  // Pass the message to the ImGui Win32 handler
        return true;  // Return true if ImGui handled the message

    switch (msg)  // Switch on the message type
    {
    case WM_SIZE:  // If the window is resized
        if (wParam == SIZE_MINIMIZED)  // If the window is minimized
            return 0;  // Do nothing
        g_ResizeWidth = (UINT)LOWORD(lParam);  // Queue the resize width
        g_ResizeHeight = (UINT)HIWORD(lParam);  // Queue the resize height
        return 0;  // Return 0 to indicate the message was handled
    case WM_SYSCOMMAND:  // If a system command is received
        if ((wParam & 0xfff0) == SC_KEYMENU)  // If the ALT key menu is activated
            return 0;  // Disable the menu
        break;  // Continue to default handling
    case WM_DESTROY:  // If the window is destroyed
        ::PostQuitMessage(0);  // Post a quit message
        return 0;  // Return 0 to indicate the message was handled
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);  // Call the default window procedure
}
