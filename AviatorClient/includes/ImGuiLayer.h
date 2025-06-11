/**
 * @file ImGuiLayer.h
 * @brief Defines the ImGuiLayer class that manages the lifecycle of the ImGui-based chat UI application.
 */

#pragma once

#include <memory>
#include <windows.h>
#include "ChatUI.h"

/**
 * @class ImGuiLayer
 * @brief Manages initialization, main loop, and cleanup of the ImGui-based GUI application.
 */
class ImGuiLayer {
public:
    /**
     * @brief Runs the main application loop.
     *
     * Initializes the window, sets up the rendering context, and enters the main loop
     * of the chat-based UI using Dear ImGui.
     *
     * @param hInstance Handle to the current application instance.
     * @param host Aviator service host.
     * @param tenant_id Tenant identifier.
     * @param client Client ID for the conversation.
     * @param tag Tag for message source/context.
     * @param aviator_model Name of the Aviator model to use.
     * @param user_name Username for authentication.
     * @param password Password for authentication.
     * @param min_log_level Minimum logging level for internal logging.
     */
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
    /**
     * @brief Initializes the application window and graphics context.
     *
     * @param hInstance Handle to the application instance.
     * @param host ITOM Aviator service host.
     * @param tenant_id SMAX Tenant ID.
     * @param client Client ID (Aviator API).
     * @param tag Tag (Aviator API).
     * @param aviator_model Aviator model to use.
     * @param user_name Username for authentication.
     * @param password Password for authentication.
     * @param min_log_level Minimum log level.
     * @return True if initialization succeeded, false otherwise.
     */
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

    /**
     * @brief Cleans up resources, including the OpenGL context and ImGui state.
     */
    void Cleanup();

    /**
     * @brief Runs the main GUI loop, processing events and rendering frames.
     */
    void MainLoop();

    HWND hwnd_ = nullptr; ///< Handle to the main application window.
    HDC hdc_ = nullptr; ///< Handle to the device context for rendering.
    HGLRC gl_context_ = nullptr; ///< OpenGL rendering context.
    std::unique_ptr<smax::ChatUI> chat_ui_; ///< Chat UI logic and rendering handler.
};
