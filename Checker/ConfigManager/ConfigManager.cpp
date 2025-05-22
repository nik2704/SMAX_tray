#include "ConfigManager.h"
#include <set>
#include <stdexcept>

ConfigManager::ConfigManager(const std::string& path, DecryptFunc decryptFunc, Utf8Func utf8Func)
    : iniFile_(path), decryptFunc_(std::move(decryptFunc)), utf8Func_(std::move(utf8Func)) {
    readConfig();
}

void ConfigManager::readConfig() {
    std::lock_guard<std::mutex> lock(mtx_);

    CSimpleIniW ini;
    ini.SetUnicode();
    if (ini.LoadFile(iniFile_.c_str()) < 0) {
        throw std::runtime_error("Failed to load INI file: " + iniFile_);
    }

    period_ = std::stoi(ini.GetValue(L"Settings", L"period", L"60"));
    auto instance = ini.GetValue(L"Settings", L"instance", L"");

    auto ini_hostname = utf8Func_(ini.GetValue(instance, L"hostname", L""));
    auto ini_tenantId = utf8Func_(ini.GetValue(instance, L"tenantId", L""));

    auto userNameHexW = ini.GetValue(instance, L"userName", L"");
    userName_ = decryptFunc_(userNameHexW);
    filter_ = "Active=true and OwnedByPerson.Upn='" + userName_ + "'";

    auto tokenHexW = ini.GetValue(instance, L"token", L"");
    token_ = decryptFunc_(tokenHexW);

    url_ = "https://" + ini_hostname + "/rest/" + ini_tenantId + "/ems/Request?layout=Id";
    portalURL_ = "https://" + ini_hostname + "/saw/Requests?TENANTID=" + ini_tenantId;
}

std::wstring ConfigManager::getValue(const std::string& section, const std::string& key) const {
    std::lock_guard<std::mutex> lock(mtx_);

    CSimpleIniW ini;
    ini.SetUnicode();
    ini.LoadFile(iniFile_.c_str());

    return ini.GetValue(std::wstring(section.begin(), section.end()).c_str(),
                        std::wstring(key.begin(), key.end()).c_str(), L"");
}

std::vector<std::string> ConfigManager::getCustomerSections() const {
    std::lock_guard<std::mutex> lock(mtx_);

    std::vector<std::string> sections;
    CSimpleIniW ini;
    ini.SetUnicode();
    ini.LoadFile(iniFile_.c_str());

    CSimpleIniW::TNamesDepend keys;
    ini.GetAllSections(keys);
    for (const auto& item : keys) {
        std::string section = utf8Func_(item.pItem);
        if (section.rfind("CUSTOMER", 0) == 0) {
            sections.push_back(section);
        }
    }
    return sections;
}

std::string ConfigManager::getUrl() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return url_;
}

std::string ConfigManager::getPortalURL() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return portalURL_;
}

std::string ConfigManager::getUserName() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return userName_;
}

std::string ConfigManager::getToken() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return token_;
}

std::string ConfigManager::getFilter() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return filter_;
}

int ConfigManager::getPeriod() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return period_;
}
