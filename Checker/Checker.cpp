// Checker.cpp
#include "Checker.h"
#include "../Utils/Utils.h"
#include "../ConfigManager/ConfigInitializer/ConfigInitializer.h"
#include "NetworkClient/NetworkClient.h"
#include "../resource.h"
#include <sstream>
#include <iomanip>

#pragma comment(lib, "wininet.lib")


namespace smax {

Checker& Checker::getInstance() {
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        ConfigInitializer::initializeToken(L"config.ini");
    });

    static Checker instance;
    return instance;
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

void Checker::start(HINSTANCE hInstance, const std::wstring& iniFile) {
    if (worker_ || tray_ != nullptr) {
        return;
    }

    iniFile_ = iniFile;
    hInst_ = hInstance;
    readConfig();

    tray_ = std::make_unique<smax::TrayManager>();
    tray_->initialize(hInstance);
    tray_->setOnAcknowledge([this]() { this->acknowledge(); });
    tray_->setOnShutdown([this]() { this->shutdown(); });
    tray_->setOnUpdateConfig([this]() { this->updateConfiguration(); });

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

void Checker::acknowledge() {
    dismissAlert();
    tray_->setIcon(LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_INIT)));

    const auto& portalURL = Checker::getInstance().getConfig()->getPortalURL();
    auto wURL = std::wstring(portalURL.begin(), portalURL.end());

    tray_->openURLinBrowser(wURL);
}

void Checker::updateConfiguration() {
    dismissAlert();

    ConfigInitializer::UpdateINI(L"config.ini");
    readConfig();
}

LRESULT CALLBACK Checker::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    std::wstring wURL;

    switch (uMsg) {
        case WM_USER + 1:
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 3, L"Settings");
                AppendMenu(hMenu, MF_STRING, 2, L"Acknowledge");
                AppendMenu(hMenu, MF_STRING, 1, L"Shut Down");

                SetForegroundWindow(hwnd);
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);

                if (cmd == 1) {
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                } else if (cmd == 2) {
                    const auto& portalURL = Checker::getInstance().getConfig()->getPortalURL();
                    wURL = std::wstring(portalURL.begin(), portalURL.end());
                    ShellExecute(hwnd, L"open", wURL.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                    Checker::getInstance().acknowledge();
                } else if (cmd == 3) {
                    Checker::getInstance().updateConfiguration();
                }
            }
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            PostQuitMessage(0);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void Checker::readConfig() {
    std::string iniPath = wideToUtf8(iniFile_.c_str());
    std::string errorMsg;

    if (!config_) {
        config_ = std::make_shared<ConfigManager>(
            iniPath, 
            [](const std::wstring& wstr) -> std::string {
                return getDecryptedString(wstr.c_str());
            },
            [this](const std::wstring& wstr) -> std::string {
                return wideToUtf8(wstr.c_str());
            },
            errorMsg
        );
    } else {
        config_->readConfig(errorMsg);
    }
    
    if (!config_->hasConfig()) {
        tray_->showErrorMessage(L"Error", utf8ToWide(errorMsg));
        tray_->setIcon(LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ERROR)));
    }
}

ConfigManager* Checker::getConfig() const {
    return config_.get();
}

} // namespace smax