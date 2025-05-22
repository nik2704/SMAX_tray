/// @file Checker.h
/// @brief Declares the smax::Checker class responsible for periodic GET requests and system tray notifications.

#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "ConfigManager/ConfigManager.h"
#include "TrayManager/TrayManager.h"
#include "Worker/Worker.h"
#include <atomic>
#include <string>
#include <set>
#include <vector>
#include <windows.h>

namespace smax {

/**
 * @class Checker
 * @brief Singleton class responsible for background monitoring and user notifications.
 *
 * The Checker class periodically sends GET requests to a specified URL, parses the response,
 * tracks processed IDs, and shows system tray notifications for new events. It is configured
 * via a specified INI file.
 */
class Checker {
public:
    /**
     * @brief Gets the singleton instance of the Checker class.
     * @return Reference to the Checker instance.
     */
    static Checker& getInstance();

    /**
     * @brief Acknowledges and clears current notifications or events.
     */
    void acknowledge();

    /**
     * @brief Updates currently used configuration in the INI file.
     */
    void updateConfiguration();

    /**
     * @brief Starts the background checker using the provided instance handle and configuration file.
     * @param hInstance Handle to the application instance.
     * @param iniFile Path to the INI configuration file.
     */
    void start(HINSTANCE hInstance, const std::wstring& iniFile);

    /**
     * @brief Gracefully stops the checker and cleans up resources.
     */
    void stop();

private:
    /// @brief Private constructor for singleton.
    Checker();

    /// @brief Destructor.
    ~Checker();

    /// @brief Deleted copy constructor.
    Checker(const Checker&) = delete;

    /// @brief Deleted assignment operator.
    Checker& operator=(const Checker&) = delete;

    /**
     * @brief Reads configuration parameters from the INI file.
     */
    void readConfig();

    /**
     * @brief Get configuration parameters from the INI file.
     */
    ConfigManager* getConfig() const;

    // /**
    //  * @brief Runs the cycle to get information from SMAX.
    //  */
    // void runWorker();
    
    /**
     * @brief Dismisses the current alert and resets the notification icon.
     */
    void dismissAlert();

    /**
     * @brief Encodes a string for safe use in a URL.
     * @param value The input string to encode.
     * @return URL-encoded string.
     */
    std::string urlEncode(const std::string& value);

    /**
     * @brief Windows message handler for the hidden notification window.
     */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    std::wstring iniFile_;                  ///< Path to the INI configuration file.
    std::unique_ptr<TrayManager> tray_;     ///< Tray manager for system tray notifications.
    HINSTANCE hInst_ = nullptr;             ///< Application instance handle.
    std::unique_ptr<Worker> worker_;        ///< Background worker for periodic tasks.
    std::shared_ptr<ConfigManager> config_; ///< Responsible for reading and holding configuration values.
};

} // namespace smax
