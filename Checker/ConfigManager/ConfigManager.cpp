#include "ConfigManager.h"


namespace smax {

ConfigManager::ConfigManager(const std::string& path, DecryptFunc decryptFunc, Utf8Func utf8Func, std::string errorMsg)
    : iniFile_(path), decryptFunc_(std::move(decryptFunc)), utf8Func_(std::move(utf8Func)) {
    readConfig(errorMsg);
}

bool ConfigManager::hasConfig() const {
    return has_cfg_;
}

bool ConfigManager::readConfig(std::string& errorMsg) {
    std::lock_guard<std::mutex> lock(mtx_);
    has_cfg_ = false;

    CSimpleIniW ini;
    ini.SetUnicode();
    if (ini.LoadFile(iniFile_.c_str()) < 0) {
        errorMsg = "Failed to load INI file: " + iniFile_; 

        return has_cfg_;
    }

    try {
        period_ = std::stoi(ini.GetValue(L"Settings", L"period", L"60"));
    } catch (const std::exception& e) {
        errorMsg = "Invalid period value: " + std::string(e.what());

        return has_cfg_;
    }

    auto instance = ini.GetValue(L"Settings", L"instance", L"");

    auto ini_hostname = utf8Func_(ini.GetValue(instance, L"hostname", L""));
    auto ini_tenantId = utf8Func_(ini.GetValue(instance, L"tenantId", L""));

    auto userNameHexW = ini.GetValue(instance, L"userName", L"");
    userName_ = decryptFunc_(userNameHexW);

    request_filter_ = "Active=true and OwnedByPerson.Upn='" + userName_ + "'";
    task_filter_ = "PlatformTaskType='ManualTask' and PhaseId!='Completed' and PhaseId!='Failed' and PhaseId!='Cancel' and Assignee.Upn='" + userName_ + "'";
    approval_filter_ = "PlatformTaskType='Approval' and PhaseId='Pending' and Assignee.Upn='" + userName_ + "'";

    auto tokenHexW = ini.GetValue(instance, L"token", L"");
    token_ = decryptFunc_(tokenHexW);

    request_url_ = "https://" + ini_hostname + "/rest/" + ini_tenantId + "/ems/Request?layout=Id";
    task_url_ = "https://" + ini_hostname + "/rest/" + ini_tenantId + "/ems/Task?layout=Id";
    approval_url_ = "https://" + ini_hostname + "/rest/" + ini_tenantId + "/ems/Task?layout=Id";

    portalURLInbox_ = "https://" + ini_hostname + "/home/inbox?TENANTID=" + ini_tenantId;
    portalURLRequests_ = "https://" + ini_hostname + "/saw/Requests?TENANTID=" + ini_tenantId;
    portalURLTasks_ = "https://" + ini_hostname + "/home/tasks?TENANTID=" + ini_tenantId;
    portalURLApprovals_ = "https://" + ini_hostname + "/home/approval?TENANTID=" + ini_tenantId;

    has_cfg_ = true;

    check_requests_ = ini.GetBoolValue(instance, L"check_requests", false);
    check_tasks_ = ini.GetBoolValue(instance, L"check_tasks", false);
    check_approvals_ = ini.GetBoolValue(instance, L"check_approvals", false);
    aviator_enabled_ = ini.GetBoolValue(instance, L"aviator_enabled", false);

    aviator_model_ = utf8Func_(ini.GetValue(L"Settings", L"aviator_model", L""));
    min_log_level_ = utf8Func_(ini.GetValue(L"Settings", L"min_log_level", L""));

    return has_cfg_;
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

std::string ConfigManager::getRequestUrl() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return request_url_;
}

std::string ConfigManager::getTaskUrl() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return task_url_;
}

std::string ConfigManager::getApprovalUrl() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return approval_url_;
}

std::string ConfigManager::getPortalURLInbox() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return portalURLInbox_;
}

std::string ConfigManager::getPortalURLRequests() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return portalURLRequests_;
}

std::string ConfigManager::getPortalURLTasks() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return portalURLTasks_;
}

std::string ConfigManager::getPortalURLApprovals() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return portalURLApprovals_;
}

std::string ConfigManager::getUserName() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return userName_;
}

std::string ConfigManager::getToken() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return token_;
}

std::string ConfigManager::getRequestFilter() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return request_filter_;
}

std::string ConfigManager::getTaskFilter() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return task_filter_;
}

std::string ConfigManager::getApprovalFilter() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return approval_filter_;
}

bool ConfigManager::getCheckRequests() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return check_requests_;
}

bool ConfigManager::getCheckTasks() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return check_tasks_;
}

bool ConfigManager::getCheckApprovals() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return check_approvals_;
}

int ConfigManager::getPeriod() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return period_;
}

std::string ConfigManager::getAviatorModel() {
    return aviator_model_;
}

std::string ConfigManager::getMinLogLevel() {
    return min_log_level_;
}

bool ConfigManager::isAviatorEnabled() {
    return aviator_enabled_;
}

} // namespace smax