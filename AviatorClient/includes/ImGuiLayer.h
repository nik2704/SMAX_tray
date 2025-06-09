#pragma once

#include <memory>
#include <windows.h>
#include "ChatUI.h"


class ImGuiLayer {
public:
    void Run(
        HINSTANCE hInstance,
        const std::string & host,
        const std::string & tenant_id,
        const std::string & client,
        const std::string & tag,
        const std::string & aviator_model,
        const std::string & user_name,
        const std::string & password,
        const std::string & min_log_level
    );

private:
    bool Init(
        HINSTANCE hInstance,
        const std::string & host,
        const std::string & tenant_id,
        const std::string & client,
        const std::string & tag,
        const std::string & aviator_model,
        const std::string & user_name,
        const std::string & password,
        const std::string & min_log_level
    );
    void Cleanup();
    void MainLoop();

    HWND hwnd_ = nullptr;
    HDC hdc_ = nullptr;
    HGLRC gl_context_ = nullptr;
    std::unique_ptr<smax::ChatUI> chat_ui_;
};
