/**
 * @file ImGuiLayer.h
 * @brief Singleton that manages the lifecycle of the ImGui-based chat UI application.
 */

#pragma once

#include <memory>
#include <windows.h>
#include <string>
#include "ChatUI.h"

class ImGuiLayer {
public:
    /**
     * @brief Gets the singleton instance of ImGuiLayer.
     * @return Reference to the ImGuiLayer instance.
     */
    static ImGuiLayer& Instance();

    /**
     * @brief Deleted copy constructor.
     */
    ImGuiLayer(const ImGuiLayer&) = delete;

    /**
     * @brief Deleted copy assignment.
     */
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    /**
     * @brief Runs the main application loop.
     */
    void Run(
        HINSTANCE hInstance,
        const std::string& host,
        const std::string& tenant_id,
        const std::string& client,
        const std::string& tag,
        const std::string& aviator_model,
        const std::string& user_name,
        const std::string& password,
        const std::string& min_log_level
    );

    void OnWindowClose();

private:
    ImGuiLayer() = default;
    ~ImGuiLayer() = default;

    bool Init(
        HINSTANCE hInstance,
        const std::string& host,
        const std::string& tenant_id,
        const std::string& client,
        const std::string& tag,
        const std::string& aviator_model,
        const std::string& user_name,
        const std::string& password,
        const std::string& min_log_level
    );

    void Cleanup();
    void MainLoop();

    HWND hwnd_ = nullptr;
    HDC hdc_ = nullptr;
    HGLRC gl_context_ = nullptr;
    std::unique_ptr<smax::ChatUI> chat_ui_;
};

