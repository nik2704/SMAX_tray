#pragma once

#include <string>
#include <windows.h>
#include <functional>

namespace smax {

/**
 * @brief Manages the system tray icon, notifications, and user interactions.
 *
 * This class encapsulates functionality related to creating a tray icon,
 * displaying balloon notifications, handling user interactions (such as
 * context menu commands), and triggering callbacks for specific actions.
 */
class TrayManager {
public:
    /**
     * @brief Constructs a TrayManager instance.
     */
    TrayManager();

    /**
     * @brief Destroys the TrayManager instance and performs cleanup.
     */
    ~TrayManager();

    /**
     * @brief Initializes the tray icon and internal window.
     * @param hInstance Handle to the application instance.
     */
    void initialize(HINSTANCE hInstance);

    /**
     * @brief Shuts down the tray icon and related resources.
     */
    void shutdown();

    /**
     * @brief Displays an informational balloon notification.
     * @param message The message text to display.
     * @param title The title of the notification balloon (default: "Notification").
     */
    void showInfo(const std::wstring& message, const std::wstring& title = L"Notification");

    /**
     * @brief Displays an Error balloon notification.
     * @param message The message text to display.
     * @param title The title of the notification balloon (default: "Notification").
     */
    void showErrorMessage(const std::wstring& title, const std::wstring& message);
    
    /**
     * @brief Sets the icon used for the tray.
     * @param hIcon Handle to the icon to display in the tray.
     */
    void setIcon(HICON hIcon);

    /**
     * @brief Dismisses the currently shown balloon alert, if any.
     */
    void dismissAlert();

    /**
     * @brief Registers a callback to be called when the user acknowledges a notification.
     * @param callback The function to call on acknowledge.
     */
    void setOnAcknowledge(std::function<void()> callback);

    /**
     * @brief Registers a callback to be called when the shutdown option is selected.
     * @param callback The function to call on shutdown.
     */
    void setOnShutdown(std::function<void()> callback);

    /**
     * @brief Registers a callback to be called when the "Update Config" action is selected.
     * @param callback The function to call on config update.
     */
    void setOnUpdateConfig(std::function<void()> callback);

    /**
     * @brief Open URL in a web browser.
     * @param url The URL.
     */
    void openURLinBrowser(const std::wstring& url);

    /**
     * @brief Invokes the acknowledgment callback, if set.
     */
    void acknowledge();

    /**
     * @brief Updates the tray icon tooltip based on request/task/approval counts.
     * @param requests Number of new requests.
     * @param tasks Number of new tasks.
     * @param approvals Number of new approvals.
     */
    void updateTooltip(int requests, int tasks, int approvals);    

private:
    HWND hwnd_; ///< Handle to the hidden window for message processing
    HINSTANCE hInstance_; /// Handle to the application instance.
    NOTIFYICONDATA nid_; /// Tray icon data structure.
    std::function<void()> onAcknowledge_; /// Callback for when a notification is acknowledged.
    std::function<void()> onShutdown_; /// Callback for when the application is requested to shut down.
    std::function<void()> onUpdateConfig_; /// Callback for when the configuration should be updated.

    /**
     * @brief Creates the tray icon and registers it with the system.
     */
    void createTrayIcon();

    /**
     * @brief Removes the tray icon from the system.
     */
    void destroyTrayIcon();
    
    /**
     * @brief Window procedure for processing tray-related messages.
     * @param hwnd Handle to the window.
     * @param uMsg The message ID.
     * @param wParam Additional message information.
     * @param lParam Additional message information.
     * @return Result of message processing.
     */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// Static instance pointer for message routing.
    static TrayManager* instance_;
};

} // namespace smax
