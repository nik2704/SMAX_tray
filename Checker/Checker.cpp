// Checker.cpp
#include "Checker.h"
#include "../ConfigManager/ConfigInitializer/ConfigInitializer.h"
#include "../libs/NetworkClient/NetworkClient.h"
#include "../libs/Logger/Logger.h"

#include "../resource.h"
#include <sstream>
#include <iomanip>

#pragma comment(lib, "wininet.lib")


namespace smax {

Checker& Checker::getInstanceCreated() {
    return getInstance(nullptr, nullptr, nullptr, nullptr);
}

Checker& Checker::getInstance(DecryptFunc decryptFunc, EncryptFunc encryptFunc, WideToUtf8Func wideToUtf8Func,  Utf8ToWideFunc utf8ToWideFunc) {
    static std::once_flag initFlag;
    static Checker* instance = nullptr;

    std::call_once(initFlag, [&]() {
        ConfigInitializer::initializeToken(L"config.ini", encryptFunc, wideToUtf8Func);
        instance = new Checker();
        instance->decryptFunc_ = decryptFunc;
        instance->wideToUtf8Func_ = wideToUtf8Func;
        instance->utf8ToWideFunc_ = utf8ToWideFunc;
        instance->encryptFunc_ = encryptFunc;
    });

    return *instance;
}

Checker::Checker() {}

Checker::~Checker() {
    stop();
}

std::string Checker::urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::uppercase << std::setw(2) << int(c);
            escaped << std::nouppercase;
        }
    }
    return escaped.str();
}

void Checker::setDinamicMenuOptions() {
    if (!config_->getAviatorModel().empty() && config_->isAviatorEnabled()) {
        tray_->setOnShowAviatorClient([this]() { });
    } else{
        tray_->setOnShowAviatorClient(nullptr);
    }
}

void Checker::start(HINSTANCE hInstance, const std::wstring& iniFile) {
    if (worker_ || tray_ != nullptr) {
        return;
    }

    iniFile_ = iniFile;
    hInst_ = hInstance;
    readConfig();

    tray_ = std::make_unique<smax::TrayManager>();
    tray_->initialize(hInstance);
    tray_->setOnAcknowledgeInbox([this]() { this->acknowledgeInbox(); });
    tray_->setOnAcknowledgeRequests([this]() { this->acknowledgeRequests(); });
    tray_->setOnAcknowledgeTasks([this]() { this->acknowledgeTasks(); });
    tray_->setOnAcknowledgeApprovals([this]() { this->acknowledgeApprovals(); });
    tray_->setOnShutdown([this]() { this->shutdown(); });
    tray_->setOnUpdateConfig([this]() {
        this->readConfig();
        this->updateConfiguration();
        this->setDinamicMenuOptions();
    });

    // AppLogger::Logger::getInstance().setLogFile("smax_tray_client.log");
    // AppLogger::Logger::getInstance().setMinLogLevel("DEBUG");
    // AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Tray started.");
    // AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "config_->getAviatorModel(): " + config_->getAviatorModel());
    // AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "config_->getAviatorModel(): " + config_->getMinLogLevel());

    setDinamicMenuOptions();

    tray_->setIcon(LoadIcon(hInstance, MAKEINTRESOURCE(SMAX_TRAY_ICON_INIT)));

    worker_ = std::make_unique<Worker>(hInst_, config_, tray_.get());
    worker_->start();
}

void Checker::shutdown() {
    stop();

    PostQuitMessage(0);
}

void Checker::stop() {
    if (worker_) {
        worker_->stop();
        worker_.reset();
    }

    if (tray_) {
        tray_->shutdown();
        tray_.reset();
    }
}

void Checker::dismissAlert() {
    tray_->dismissAlert();
}

void smax::Checker::acknowledgeAndOpenURL(int requests, int tasks, int approvals, std::string (ConfigManager::*getUrlFunc)() const) {
    dismissAlert();
    tray_->updateTooltip(requests, tasks, approvals);

    if (requests == 0 && tasks == 0 && approvals == 0) {
        tray_->setIcon(LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_INIT)));
    }

    const auto& instance = Checker::getInstanceCreated();

    const auto* config = instance.getConfig();
    if (config == nullptr) {
        tray_->showErrorMessage(L"Error", L"Configuration not loaded.");
        return;
    }

    const auto& portalURL = (config->*getUrlFunc)();
    auto wURL = std::wstring(portalURL.begin(), portalURL.end());

    tray_->openURLinBrowser(wURL);
}

void smax::Checker::acknowledgeInbox() {
    acknowledgeAndOpenURL(0, 0, 0, &ConfigManager::getPortalURLInbox);
}

void smax::Checker::acknowledgeRequests() {
    FetchStats stats = worker_->getFetchStats();
    worker_->resetFetschStatsRequests();
    acknowledgeAndOpenURL(0, stats.tasks, stats.approvals, &ConfigManager::getPortalURLRequests);
}

void smax::Checker::acknowledgeTasks() {
    FetchStats stats = worker_->getFetchStats();
    worker_->resetFetschStatsTasks();
    acknowledgeAndOpenURL(stats.requests, 0, stats.approvals, &ConfigManager::getPortalURLTasks);
}

void smax::Checker::acknowledgeApprovals() {
    FetchStats stats = worker_->getFetchStats();
    worker_->resetFetschStatsApprovals();
    acknowledgeAndOpenURL(stats.requests, stats.tasks, 0, &ConfigManager::getPortalURLApprovals);
}

void Checker::updateConfiguration() {
    dismissAlert();

    ConfigInitializer::UpdateINI(L"config.ini", decryptFunc_, encryptFunc_, utf8ToWideFunc_);
    readConfig();
}

void Checker::readConfig() {
    std::string iniPath = wideToUtf8Func_(iniFile_.c_str());
    std::string errorMsg;

    if (!config_) {
        config_ = std::make_shared<ConfigManager>(
            iniPath, 
            decryptFunc_,
            wideToUtf8Func_,
            errorMsg
        );
    } else {
        config_->readConfig(errorMsg);
    }
    
    if (!config_->hasConfig()) {
        tray_->showErrorMessage(L"Error", utf8ToWideFunc_(errorMsg));
        tray_->setIcon(LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ERROR)));
    }
}

ConfigManager* Checker::getConfig() const {
    return config_.get();
}

} // namespace smax