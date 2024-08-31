#include "sphere.hh"
#include "plane.hh"
#include "box.hh"
#include "camera.hh"
#include "light.hh"
#include "scene.hh"
#include "blob.hh"
#include "obj_reader.hh"
#include "triangle.hh"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <chrono>
#include <iostream>
#include <windows.h>
#include <d3d11.h>
#include <thread>

#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1920
#define HEIGHT 500
#define WIDTH 500
#define NB_SAMPLES 1
#define DEPTH 1
#define NB_THREADS 0

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

// Data
static ID3D11Device*                    g_pd3dDevice = nullptr;
static ID3D11DeviceContext*             g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*                  g_pSwapChain = nullptr;
static bool                             g_SwapChainOccluded = false;
static UINT                             g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*          g_mainRenderTargetView = nullptr;
static std::atomic<bool>                done(false);
static std::atomic<int>                 current_sample(1);
static Image                            current_image(WIDTH, HEIGHT);
static std::vector<Color>*              accumulation_buffer = new std::vector<Color>(WIDTH * HEIGHT);
static ID3D11ShaderResourceView*        texture = nullptr;
static std::chrono::duration<double>    elapsed = std::chrono::duration<double>(0);
static std::chrono::duration<double>    renderTime = std::chrono::duration<double>(0);
std::mutex cameraMutex;

//Variables
Vector3* cameraPosition = new Vector3(2, 0, 2);
Vector3* cameraDirection = new Vector3(1, 0, 0);
Vector3* cameraUp = new Vector3(0, 0, -1);
bool paused = false;
bool warning_popup = false;
bool save_image_popup = false;

//Parameters
float cameraSpeed = 0.1f;
float mouseSensitivity = 0.001f;
float yaw = -90.0f;
float pitch = 0.0f;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ComputeThread(Scene* scene) {
    while (!done) {
        if (paused)
            continue;
        auto start = std::chrono::high_resolution_clock::now();
        Image output = scene->Render(HEIGHT, WIDTH, NB_SAMPLES, DEPTH, NB_THREADS, accumulation_buffer, current_sample.load());
        current_sample.fetch_add(1);
        std::lock_guard<std::mutex> lock(current_image.pixelMutex);
        current_image = output;
        auto end = std::chrono::high_resolution_clock::now();
        elapsed = end - start;
        renderTime += elapsed;  
    }
}
void RenderUI() {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit")) {
            PostQuitMessage(0);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Options")) {
        ImGui::MenuItem("Pause Rendering", NULL, &paused);
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}
int main()
{   
    
    try {
    //Determine the number of threads
    int nb_threads = std::thread::hardware_concurrency();
    if (nb_threads == 0)
        nb_threads = 1;

    
    // Setup window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Filmo's Path Tracer", WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    Color Red = Color(0.5, 0.5, 0.5);
    Color Blue = Color(0, 0, 0.5);

    
    Camera* camera = new Camera(cameraPosition, cameraDirection, cameraUp, 20, 20, 1, 1 , 2, (float)16/9);
    Scene* scene = new Scene(camera);

    
    
    Plane *ground = new Plane(new Uniform_Texture(Color(0.4,0.4,0.4), 0.9, 0.5, 32, 0, 0), Point3(0,0,0), Vector3(0,0,1));
    scene->addObject(ground); 
    Plane *back = new Plane(new Uniform_Texture(Color(0.2,0.2,0.2), 0.9, 0.5, 32, 0, 0), Point3(15,0,0), Vector3(-1,0,0));
    scene->addObject(back); 
    Plane *left = new Plane(new Uniform_Texture(Color(0,0,1), 0.9, 0.5, 32, 0, 0), Point3(0,5,0), Vector3(0,-1,0));
    scene->addObject(left);
    Plane *right = new Plane(new Uniform_Texture(Color(1,0,0), 0.9, 0.5, 32, 0, 0), Point3(0,-5,0), Vector3(0,1,0));
    scene->addObject(right);
    Plane *top = new Plane(new Uniform_Texture(Color(0.2,0.2,0.2), 0.9, 0.5, 32, 0, 0), Point3(5,0,5), Vector3(0,0,-1));
    scene->addObject(top);
    Plane *behind = new Plane(new Uniform_Texture(Color(0, 0, 0.7), 0.9, 0.5, 32, 0, 0), Point3(0, 0, 0), Vector3(-1,0,0));
    scene->addObject(behind);   
    Sphere *glassSphere = new Sphere(new Dieletric_Texture(Color(1,1,1),1.7), Point3(8, 1, 1), 1);
    scene->addObject(glassSphere); 
    Sphere *normalSphere1 = new Sphere(new Uniform_Texture(Color(0,1,0), 0.5, 0.5, 32, 0, 0), Point3(10, -2, 1), 1);
    scene->addObject(normalSphere1);
    Sphere *metalSphere1 = new Sphere(new Metal_Texture(Color(1,1,1), 0.5), Point3(10, 0, 2), 1);
    scene->addObject(metalSphere1);
    Sphere_Light *lightSphere = new Sphere_Light(Color(1,1,1), Point3(10, 0, 104.7), 100, 1);
    scene->addObject(lightSphere);


    std::cout << "Number of objects in the scene: " << scene->objects.size() << std::endl;
    std::cout << "Rendering scene" << std::endl;
    std::cout << "Number of samples for indirect lighting: " << NB_SAMPLES << std::endl;
    std::cout << "Depth of the ray casting: " << DEPTH << std::endl;
    bool done = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ID3D11ShaderResourceView* texture = nullptr;
    std::thread compute_thread(ComputeThread, scene);
    while(!done)
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        if (texture)
        {
            texture->Release();
            texture = nullptr;
        }
        texture = current_image.CreateTextureFromImage(g_pd3dDevice);
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 windowSize = ImVec2(WIDTH, HEIGHT);
        ImVec2 windowPos = ImVec2((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
        ImGui::Begin("Filmo's Path Tracer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Average render time: %dms/frame | Sample count: %d", (int)(elapsed.count() * 1000), current_sample.load());
        if (texture)
        {
            ImGui::Image((void*)texture, ImVec2((float)WIDTH, (float)HEIGHT));
        }
        //If save image button pressed, save the image
        if (ImGui::Button("Save Image"))
        {
            if (!paused)
            {
                warning_popup = true;
            }
            else
            {
                save_image_popup = true;
            }
        }
        //Add a check box to pause the rendering
        ImGui::Checkbox("Pause", &paused);

        if (warning_popup)
        {
            ImGui::OpenPopup("Warning");
        }
        if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Please pause the rendering before saving the image");
            if (ImGui::Button("OK"))
            {
                warning_popup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (save_image_popup)
        {
            ImGui::OpenPopup("Save Image");
        }
        if (ImGui::BeginPopupModal("Save Image", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Please enter the name of the image");
            static char buffer[256] = "image.ppm";
            ImGui::InputText("Name", buffer, 256);
            if (ImGui::Button("Save"))
            {
                current_image.save(buffer);
                save_image_popup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);   
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
        std::this_thread::sleep_for(std::chrono::milliseconds(0));  // Adjust sleep time as necessary
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    delete accumulation_buffer;
    done = true;
    compute_thread.join();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    }   
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    catch(...) {
        std::cerr << "Unknown exception" << std::endl;
    }
    return 0;
}


// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
void UpdateCameraDirection(float xOffset, float yOffset) {
    // Normalize the offsets based on the mouse sensitivity
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    // Update camera direction using the mouse movement offsets
    Vector3 right = cameraDirection->cross(*cameraUp).normalize();
    Vector3 up = right.cross(*cameraDirection).normalize();

    Vector3 newDirection = *cameraDirection + (right * xOffset) + (up * yOffset);
    newDirection = newDirection.normalize();

    std::lock_guard<std::mutex> lock(cameraMutex);
    *cameraDirection = newDirection;
}

void UpdateCameraPosition(Vector3 position) {
    std::lock_guard<std::mutex> lock(cameraMutex);
    cameraPosition->x = position.x;
    cameraPosition->y = position.y;
    cameraPosition->z = position.z;
}
inline float Clamp(float x, float minVal, float maxVal) {
    return MAX(minVal, MIN(maxVal, x));
}
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    static bool isLeftMouseButtonDown = false;
    static bool firstMouse = true;
    static float lastX = 0.0f, lastY = 0.0f;
    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_LBUTTONDOWN:
        isLeftMouseButtonDown = true;  // Left mouse button is pressed
        firstMouse = true;  
        SetCapture(hWnd);  // Capture the mouse
        return 0;
    case WM_LBUTTONUP:
        isLeftMouseButtonDown = false;  // Left mouse button is released
        ReleaseCapture();  // Release the mouse
        return 0;
    case WM_MOUSEMOVE: {
        if (!isLeftMouseButtonDown)
            return 0; 

        float xpos = (float)LOWORD(lParam);
        float ypos = (float)HIWORD(lParam);

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
 
        float xOffset = xpos - lastX;
        float yOffset = ypos - lastY; // Inverted since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        UpdateCameraDirection(xOffset, yOffset);

        // Reset samples and buffer for updated view
        current_sample = 1;
        accumulation_buffer = new std::vector<Color>(WIDTH * HEIGHT);

        }
        return 0;

    case WM_KEYDOWN:
        switch (wParam) {
            case 'Z': // Move forward
                UpdateCameraPosition(Vector3(*cameraPosition + *cameraDirection * cameraSpeed));
                current_sample = 1;
                accumulation_buffer = new std::vector<Color>(WIDTH * HEIGHT);
                break;
            case 'S': // Move backward
                UpdateCameraPosition(Vector3(*cameraPosition - *cameraDirection * cameraSpeed));
                current_sample = 1;
                accumulation_buffer = new std::vector<Color>(WIDTH * HEIGHT);
                break;
            case 'Q': // Move left
                UpdateCameraPosition(Vector3(*cameraPosition - (*cameraDirection).cross(*cameraUp).normalize() * cameraSpeed));
                current_sample = 1;
                accumulation_buffer = new std::vector<Color>(WIDTH * HEIGHT);
                break;
            case 'D': // Move right
                UpdateCameraPosition(Vector3(*cameraPosition + (*cameraDirection).cross(*cameraUp).normalize() * cameraSpeed));
                current_sample = 1;
                accumulation_buffer = new std::vector<Color>(WIDTH * HEIGHT);
                break;
            case VK_ESCAPE:
                PostQuitMessage(0); // Exit the application
                break;
            case VK_SPACE:
                paused = !paused;
                break;
        }
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

