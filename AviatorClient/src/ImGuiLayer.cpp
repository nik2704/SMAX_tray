#include "ImGuiLayer.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl3.h"
#include "TextureLoader.h"

#include <gl/GL.h>
#include <iostream>

static ImTextureID userAvatarTex = 0;
static ImTextureID botAvatarTex = 0;
static const char* USER_IMG = "assets/user.png";
static const char* BOT_IMG = "assets/aviator.png";

// Singleton accessor
ImGuiLayer& ImGuiLayer::Instance() {
    static ImGuiLayer instance;
    return instance;
}

void ImGuiLayer::OnWindowClose() {
    if (chat_ui_) {
        chat_ui_->setIsShuttingDown(true);
    }
}

// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_CLOSE:
            ImGuiLayer::Instance().OnWindowClose();
            DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ImGuiLayer::Run(
    HINSTANCE hInstance,
    const std::string& host,
    const std::string& tenant_id,
    const std::string& client,
    const std::string& tag,
    const std::string& aviator_model,
    const std::string& user_name,
    const std::string& password,
    const std::string& min_log_level
) {
    if (!Init(hInstance, host, tenant_id, client, tag, aviator_model, user_name, password, min_log_level)) return;
    MainLoop();
    Cleanup();
}

bool ImGuiLayer::Init(
    HINSTANCE hInstance,
    const std::string& host,
    const std::string& tenant_id,
    const std::string& client,
    const std::string& tag,
    const std::string& aviator_model,
    const std::string& user_name,
    const std::string& password,
    const std::string& min_log_level
) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ImGuiWin32Class";
    wc.style = CS_OWNDC;
    RegisterClass(&wc);

    hwnd_ = CreateWindowEx(0, wc.lpszClassName, L"SMAX tray client", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, 1200, 810, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd_) {
        std::cerr << "Failed to create window.\n";
        return false;
    }

    hdc_ = GetDC(hwnd_);
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 8, 0,
        PFD_MAIN_PLANE, 0, 0, 0, 0
    };

    int pf = ChoosePixelFormat(hdc_, &pfd);
    if (!pf || !SetPixelFormat(hdc_, pf, &pfd)) {
        std::cerr << "Failed to set pixel format.\n";
        return false;
    }

    // ✅ Create and set OpenGL rendering context
    gl_context_ = wglCreateContext(hdc_);
    if (!gl_context_) {
        std::cerr << "Failed to create OpenGL context.\n";
        return false;
    }

    if (!wglMakeCurrent(hdc_, gl_context_)) {
        std::cerr << "Failed to make OpenGL context current.\n";
        return false;
    }


    // ✅ Now you can initialize ImGui with OpenGL
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd_);
    ImGui_ImplOpenGL3_Init("#version 130");

    // ✅ Load avatars after OpenGL is initialized
    userAvatarTex = LoadTextureFromFile(USER_IMG);
    botAvatarTex = LoadTextureFromFile(BOT_IMG);

    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);

    chat_ui_ = std::make_unique<smax::ChatUI>(
        host,
        tenant_id,
        client,
        tag,
        aviator_model,
        user_name,
        password,
        (ImTextureID)(intptr_t)userAvatarTex,
        (ImTextureID)(intptr_t)botAvatarTex,
        min_log_level
    );

    return true;
}

void ImGuiLayer::MainLoop() {
    MSG msg = {};
    while (IsWindow(hwnd_)) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (wglGetCurrentContext() != gl_context_ || wglGetCurrentDC() != hdc_) {
            wglMakeCurrent(hdc_, gl_context_);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RECT rect;
        GetClientRect(hwnd_, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        if (chat_ui_) {
            chat_ui_->Render();
        }

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(hdc_);
    }
}

void ImGuiLayer::Cleanup() {
    if (userAvatarTex) {
        GLuint texId = static_cast<GLuint>(userAvatarTex);
        glDeleteTextures(1, &texId);
        userAvatarTex = 0;
    }

    if (botAvatarTex) {
        GLuint texId = static_cast<GLuint>(botAvatarTex);
        glDeleteTextures(1, &texId);
        botAvatarTex = 0;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (gl_context_) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(gl_context_);
        gl_context_ = nullptr;
    }

    if (gl_context_) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(gl_context_);
        gl_context_ = nullptr;
    }

    if (hdc_ && hwnd_) {
        ReleaseDC(hwnd_, hdc_);
        hdc_ = nullptr;
    }

    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }

    chat_ui_.reset();
}
