#pragma once

#include <functional>
#include <string>
#include <vector>
#include <mutex>
#include <SimpleIni.h>

class ConfigManager {
public:
    using DecryptFunc = std::function<std::string(const std::wstring&)>;
    using Utf8Func    = std::function<std::string(const std::wstring&)>;

    ConfigManager(const std::string& path, DecryptFunc decryptFunc, Utf8Func utf8Func);

    std::wstring getValue(const std::string& section, const std::string& key) const;
    std::vector<std::string> getCustomerSections() const;

    std::string getUrl() const;
    std::string getPortalURL() const;
    std::string getUserName() const;
    std::string getToken() const;
    std::string getFilter() const;
    int getPeriod() const;

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;    

private:
    std::string iniFile_;
    DecryptFunc decryptFunc_;
    Utf8Func utf8Func_;
    std::string url_;                       ///< URL to fetch data from.
    std::string portalURL_;                 ///< Base portal URL for link.
    std::string userName_;                  ///< User name used for API authentication or identification.
    std::string token_;                     ///< API token for authorization.
    std::string filter_;                    ///< Optional filter string for requests.
    int period_ = 60;                       ///< Period in seconds between each request.

    void readConfig();

    mutable std::mutex mtx_;
};
