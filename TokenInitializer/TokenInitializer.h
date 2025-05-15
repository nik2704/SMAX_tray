#pragma once

#include <windows.h>
#include <string>
#include "../Utils/SimpleIni.h"

namespace smax {

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
class TokenInitializer {
public:
    /**
     * @brief Initializes a user token if not already set in the INI configuration (if token=-init-).
     *
     * @param iniPath Full path to the INI file used for configuration.
     */
    static void initializeToken(const std::wstring& iniPath);

private:
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
