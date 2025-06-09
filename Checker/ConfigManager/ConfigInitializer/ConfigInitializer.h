#pragma once

#include <functional>
#include <string>
#include <windows.h>
#include "../SimpleIni.h"

namespace smax {

/**
 * @brief Structure to hold input data to create an ini file.
 */
struct InputFullData {
    wchar_t hostname[256];
    wchar_t tenant[256];
    wchar_t client[256];
    wchar_t tag[256];
    wchar_t period[256];
    wchar_t username[256];
    wchar_t token[256];
    int check_requests = 1;
    int check_tasks = 1;
    int check_approvals = 1;
    int aviator_enabled = 1;    
};

/**
 * @brief Structure to hold input data for username and token.
 */
struct InputData {
    wchar_t* username; ///< Pointer to a buffer that will receive the username input.
    wchar_t* token;    ///< Pointer to a buffer that will receive the token input.
};

/**
 * @class TokenInitializer
 * @brief Provides functionality to initialize and save user tokens from a configuration file.
 *
 * This class reads a token placeholder from an INI file, prompts the user for
 * authentication credentials (username and token), and updates the file with encrypted data.
 */
class ConfigInitializer {
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
    using WideToUtf8Func = std::function<std::string(const std::wstring&)>;

    /**
     * @brief Function type alias for converting UTF-8 strings to wide strings.
     */
    using Utf8ToWideFunc = std::function<std::wstring(const std::string&)>;

    /**
     * @brief Initializes a user token if not already set in the INI configuration (if token=-init-).
     * @param encryptFunc Function to encrypt wide strings.
     * @param wideToUtf8Func Function to convert wide strings to UTF-8 strings.
     * @param iniPath Full path to the INI file used for configuration.
     */
    static void initializeToken(const std::wstring& iniPath, EncryptFunc encryptFunc, WideToUtf8Func wideToUtf8Func);

    /**
     * @brief Updates information in the INI file.
     * @param decryptFunc Function to decrypt wide strings.
     * @param encryptFunc Function to encrypt wide strings.
     * @param utf8ToWideFunc Function to convert UTF-8 strings to wide strings.
     * @param iniPath Full path to the INI file used for configuration.
     */
    static void UpdateINI(const std::wstring& iniPath, DecryptFunc decryptFunc, EncryptFunc encryptFunc, Utf8ToWideFunc utf8ToWideFunc);

private:
    /**
     * @brief Runs a dialog to create the INI file.
     * @param iniPath Full path to the INI file used for configuration.
     * @param encryptFunc Function to encrypt wide strings.
     */
    static bool generateINI(const std::wstring& iniPath, EncryptFunc encryptFunc);

    /**
     * @brief Processes the INI file to extract and update user credentials.
     * @param iniPath Full path to the INI file used for configuration.
     * @param encryptFunc Function to encrypt wide strings.
     */    
    static void processINI(const std::wstring& iniPath, EncryptFunc encryptFunc);

    /**
     * @brief Prompts the user for both username and token using a modal dialog.
     *
     * Returns a pair of strings representing the username and token. If the user cancels,
     * both strings will be empty.
     *
     * @return A std::pair containing the username and token.
     */
    static std::pair<std::wstring, std::wstring> promptInput();
};

} // namespace smax