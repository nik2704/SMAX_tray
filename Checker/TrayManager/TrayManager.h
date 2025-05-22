#pragma once

#include <string>
#include <windows.h>
#include <functional>

namespace smax {

/**
 * @brief Manages the system tray icon, notifications, and user interactions.
 */
class TrayManager {
public:
    TrayManager();
    ~TrayManager();

    void initialize(HINSTANCE hInstance);
    void shutdown();

    void showInfo(const std::wstring& message, const std::wstring& title = L"Notification");
    void setIcon(HICON hIcon);
    void dismissAlert();

    void setOnAcknowledge(std::function<void()> callback);
    void setOnShutdown(std::function<void()> callback);
    void setOnUpdateConfig(std::function<void()> callback);

    void acknowledge();

private:
    HWND hwnd_;
    HINSTANCE hInstance_;
    NOTIFYICONDATA nid_;

    std::function<void()> onAcknowledge_;
    std::function<void()> onShutdown_;
    std::function<void()> onUpdateConfig_;

    void createTrayIcon();
    void destroyTrayIcon();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static TrayManager* instance_;
};

} // namespace smax
