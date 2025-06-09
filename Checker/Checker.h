/// @file Checker.h
/// @brief Declares the smax::Checker class responsible for periodic GET requests and system tray notifications.

#pragma once


#include "ConfigManager/ConfigManager.h"
#include "TrayManager/TrayManager.h"
#include "Worker/Worker.h"
#include <string>


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
     * @brief Function type alias for decrypting wide string inputs to UTF-8 strings.
     */
    using DecryptFunc = std::function<std::string(const std::wstring&)>;

    /**
     * @brief Function type alias for encrypting wide string inputs.
     */
    using EncryptFunc = std::function<std::wstring(std::wstring&&)>;

    /**
     * @brief Function type alias for converting wide strings to UTF-8 strings.
     */
    using WideToUtf8Func    = std::function<std::string(const std::wstring&)>;

    /**
     * @brief Function type alias for converting UTF-8 strings to wide strings.
     */
    using Utf8ToWideFunc = std::function<std::wstring(const std::string&)>;

    /**
     * @brief Gets the singleton instance of the Checker class.
     * @param decryptFunc Function to decrypt wide strings.
     * @param utf8Func Function to convert wide strings to UTF-8 strings.
     * @return Reference to the Checker instance.
     */
    static Checker& getInstance(DecryptFunc decryptFunc, EncryptFunc encryptFunc, WideToUtf8Func wideToUtf8Func, Utf8ToWideFunc utf8ToWideFunc);

    /**
     * @brief Gets the instance that was inialized by the getInstance(DecryptFunc decryptFunc, Utf8Func utf8Func).
     * @return Reference to the Checker instance.
     */    
    static Checker& getInstanceCreated();

    /**
     * @brief Acknowledges and clears current notifications or events.
     */
    void acknowledgeAndOpenURL(int requests, int tasks, int approvals, std::string (ConfigManager::*getUrlFunc)() const);
    void acknowledgeInbox();
    void acknowledgeRequests();
    void acknowledgeTasks();
    void acknowledgeApprovals();

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
     * @brief Gracefully shutdown the application.
     */
    void shutdown();

private:
    Checker();  /// @brief Private constructor for singleton.

    ~Checker(); /// @brief Destructor.

    Checker(const Checker&) = delete;   /// @brief Deleted copy constructor.
    
    Checker& operator=(const Checker&) = delete;    /// @brief Deleted assignment operator.

    void setDinamicMenuOptions();

    /**
     * @brief Gracefully stop the checker and cleans up resources.
     */
    void stop();

    /**
     * @brief Reads configuration parameters from the INI file.
     */
    void readConfig();

    /**
     * @brief Get configuration parameters from the INI file.
     */
    ConfigManager* getConfig() const;
    
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
    DecryptFunc decryptFunc_ = nullptr;     ///< Function to decrypt wide strings.
    WideToUtf8Func wideToUtf8Func_ = nullptr;           ///< Function to convert wide strings to UTF-8.    
    Utf8ToWideFunc utf8ToWideFunc_ = nullptr;           ///< Function to convert UTF-8 strings to wide.
    EncryptFunc encryptFunc_ = nullptr;     ///< Function to encrypt wide strings.
};

} // namespace smax
