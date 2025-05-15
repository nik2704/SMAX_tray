/// @file Checker.h
/// @brief Declares the smax::Checker class responsible for periodic GET requests and system tray notifications.

#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <thread>
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
     * @brief Starts the background checker using the provided instance handle and configuration file.
     * @param hInstance Handle to the application instance.
     * @param iniFile Path to the INI configuration file.
     */
    void start(HINSTANCE hInstance, const std::wstring& iniFile);

    /**
     * @brief Gracefully stops the checker and cleans up resources.
     */
    void shutdown();

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
     * @brief Encodes a string for safe use in a URL.
     * @param value The input string to encode.
     * @return URL-encoded string.
     */
    std::string urlEncode(const std::string& value);

    /**
     * @brief Converts a wide-character string to UTF-8.
     * @param wstr Pointer to the wide-character string.
     * @return UTF-8 encoded string.
     */
    std::string to_utf8(const wchar_t* wstr);

    /**
     * @brief Parses a JSON string to extract a list of IDs.
     * @param json_str The JSON-formatted string.
     * @return Vector of extracted ID strings.
     */
    std::vector<std::string> extract_ids_from_json(const std::string& json_str);

    /**
     * @brief Updates the set of already processed IDs.
     * @param ids The list of IDs from the latest fetch.
     * @return Number of new IDs that were not previously processed.
     */
    size_t update_processed_ids(const std::vector<std::string>& ids);

    /**
     * @brief Sends a GET request to the specified URL.
     * @param url The full URL to request.
     */
    void sendGET(const std::string& url);

    /**
     * @brief Displays a system tray notification with the given message.
     * @param message The message to display in the notification.
     */
    void showNotification(const std::string& message);

    /**
     * @brief Windows message handler for the hidden notification window.
     */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    std::wstring iniFile_;                  ///< Path to the INI configuration file.
    HWND hwnd_ = nullptr;                   ///< Handle to the hidden window.
    HINSTANCE hInst_ = nullptr;             ///< Application instance handle.
    NOTIFYICONDATA nid_ = {};               ///< Notification icon data.
    std::thread worker_;                    ///< Background worker thread.
    std::atomic<bool> running_;             ///< Flag indicating if the checker is running.
    std::set<std::string> processedIDs_;    ///< Set of already processed IDs to avoid duplicates.

    std::string url_;                       ///< URL to fetch data from.
    std::string portalURL_;                 ///< Base portal URL for link.
    std::string userName_;                  ///< User name used for API authentication or identification.
    std::string token_;                     ///< API token for authorization.
    std::string filter_;                    ///< Optional filter string for requests.
    int period_ = 60;                       ///< Period in seconds between each request.

    static Checker* instance_;              ///< Singleton instance pointer.
};

} // namespace smax
