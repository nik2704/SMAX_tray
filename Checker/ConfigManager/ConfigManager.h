#pragma once

#include <functional>
#include <string>
#include <vector>
#include <mutex>
#include "SimpleIni.h"

namespace smax {
/**
 * @class ConfigManager
 * @brief Manages configuration loading and provides access to configuration parameters.
 * 
 * This class reads configuration from an INI file, supports decrypting and UTF-8 conversion
 * of string values, and provides accessors for commonly used config settings.
 */
class ConfigManager {
public:
    /**
     * @brief Function type alias for decrypting wide string inputs to UTF-8 strings.
     */
    using DecryptFunc = std::function<std::string(const std::wstring&)>;

    /**
     * @brief Function type alias for converting wide strings to UTF-8 strings.
     */
    using Utf8Func    = std::function<std::string(const std::wstring&)>;

    /**
     * @brief Constructs a ConfigManager.
     * 
     * @param path Path to the INI configuration file.
     * @param decryptFunc Function to decrypt wide strings.
     * @param utf8Func Function to convert wide strings to UTF-8 strings.
     */
    ConfigManager(const std::string& path, DecryptFunc decryptFunc, Utf8Func utf8Func, std::string errorMsg);

    /**
     * @brief Retrieves a value from the configuration by section and key.
     * 
     * @param section The section name in the INI file.
     * @param key The key name within the section.
     * @return The corresponding value as a wide string.
     */
    std::wstring getValue(const std::string& section, const std::string& key) const;

    /**
     * @brief Retrieves all customer-specific sections from the configuration.
     * 
     * @return Vector of section names corresponding to customers.
     */
    std::vector<std::string> getCustomerSections() const;

    /**
     * @brief Gets the URL to fetch data from.
     * @return The URL string.
     */
    std::string getUrl() const;

    /**
     * @brief Gets the base portal URL.
     * @return The portal URL string.
     */
    std::string getPortalURL() const;

    /**
     * @brief Gets the username for API authentication.
     * @return The user name string.
     */
    std::string getUserName() const;

    /**
     * @brief Gets the API token for authorization.
     * @return The token string.
     */
    std::string getToken() const;

    /**
     * @brief Gets the optional filter string for requests.
     * @return The filter string.
     */
    std::string getFilter() const;

    /**
     * @brief Gets the period in seconds between requests.
     * @return The period integer.
     */
    int getPeriod() const;

    /**
     * @brief Reads and parses the configuration from the INI file.
     * @param errorMsg Error message.
     * @return true if it was sucessful.
     */
    bool readConfig(std::string& errorMsg);

    /**
     * @brief Result of load CFG.
     * @return true if cfg is loaded.
     */
    bool hasConfig() const;

    // Disable copy construction and assignment
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;    

private:
    std::string iniFile_;          ///< Path to the INI configuration file.
    DecryptFunc decryptFunc_;      ///< Function to decrypt wide strings.
    Utf8Func utf8Func_;            ///< Function to convert wide strings to UTF-8.
    std::string url_;              ///< URL to fetch data from.
    std::string portalURL_;        ///< Base portal URL for link.
    std::string userName_;         ///< User name used for API authentication or identification.
    std::string token_;            ///< API token for authorization.
    std::string filter_;           ///< Optional filter string for requests.
    int period_ = 60;              ///< Period in seconds between each request.
    bool has_cfg_ = false;         ///< Flag indicating if the configuration was successfully loaded.;

    mutable std::mutex mtx_;       ///< Mutex for thread-safe access.
};

} // namespace smax