// Checker.cpp
#include "Checker.h"
#include "../Utils/SimpleIni.h"
#include "../Utils/Utils.h"
#include "../TokenInitializer/TokenInitializer.h"
#include "NetworkClient/NetworkClient.h"
#include "../resource.h"

#include <shellapi.h>
#include <wininet.h>
#include <sstream>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <Windows.h>

#pragma comment(lib, "wininet.lib")


STARTUPINFO si = { sizeof(STARTUPINFO) };
PROCESS_INFORMATION pi;

namespace smax {

Checker& Checker::getInstance() {
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        TokenInitializer::initializeToken(L"config.ini");
    });

    static Checker instance;
    return instance;
}

Checker::Checker() : running_(false) {}

Checker::~Checker() {
    shutdown();
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
    if (running_) {
        return;
    }

    iniFile_ = iniFile;
    hInst_ = hInstance;
    readConfig();

    const wchar_t CLASS_NAME[] = L"SMAXTrayAppClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    hwnd_ = CreateWindowEx(0, CLASS_NAME, L"SMAXTrayAppClass", 0, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd_) return;

    nid_ = {};
    nid_.cbSize = sizeof(nid_);
    nid_.hWnd = hwnd_;
    nid_.uID = 1;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_USER + 1;
    nid_.hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_INIT));
    wcscpy_s(nid_.szTip, L"SMAX requests");
    Shell_NotifyIcon(NIM_ADD, &nid_);

    running_ = true;

    runWorker();
}

void Checker::runWorker() {
    worker_ = std::thread([this]() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (running_) {
            lock.unlock();
            std::string encoded_filter = urlEncode(config_->getFilter());
            std::string smax_url = config_->getUrl() + "&filter=" + encoded_filter;
            sendGET(smax_url);
            lock.lock();

            cv_.wait_for(lock, std::chrono::seconds(config_->getPeriod()), [this]() { return !running_; });
        }
    });
}

void Checker::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();

    if (worker_.joinable()) {
        worker_.join();
    }

    Shell_NotifyIcon(NIM_DELETE, &nid_);
    DestroyWindow(hwnd_);
}

void Checker::shutdown() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }

    if (hwnd_) {
        Shell_NotifyIcon(NIM_DELETE, &nid_);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void Checker::dismissAlert() {
    nid_.uFlags |= NIF_INFO;
    wcscpy_s(nid_.szInfo, L"");
    wcscpy_s(nid_.szInfoTitle, L"");
    nid_.dwInfoFlags = NIIF_NONE;
}


void Checker::acknowledge() {
    dismissAlert();

    nid_.hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_INIT));
    Shell_NotifyIcon(NIM_MODIFY, &nid_);
}

void Checker::updateConfiguration() {
    dismissAlert();

    TokenInitializer::UpdateINI(L"config.ini");
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

    config_ = std::make_unique<ConfigManager>(
        iniPath, 
        [](const std::wstring& wstr) -> std::string {
            return getDecryptedString(wstr.c_str());
        },
        [this](const std::wstring& wstr) -> std::string {
            return wideToUtf8(wstr.c_str());
        }
    );
}

ConfigManager* Checker::getConfig() const {
    return config_.get();
}

size_t Checker::update_processed_ids(const std::vector<std::string>& ids) {
    std::set<std::string> current(ids.begin(), ids.end());

    size_t new_count = 0;
    for (const auto& id : current) {
        if (processedIDs_.find(id) == processedIDs_.end()) {
            ++new_count;
        }
    }

    for (const auto& id : current) {
        processedIDs_.insert(id);
    }

    for (auto it = processedIDs_.begin(); it != processedIDs_.end(); ) {
        if (current.find(*it) == current.end()) {
            it = processedIDs_.erase(it);
        } else {
            ++it;
        }
    }

    return new_count;
}

void Checker::sendGET(const std::string& url) {
    auto result = NetworkClient::get(url, config_->getUserName(), config_->getToken());
    dismissAlert();
    
    if (result.has_value()) {
        auto ids = NetworkClient::extractIDsFromJSON(result.value());
        auto newElements = update_processed_ids(ids);

        if (newElements > 0) {
            showNotification("New " + std::to_string(newElements) + " requests found!");
            nid_.hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ALERT));
            Shell_NotifyIcon(NIM_MODIFY, &nid_);
        }
    } else {
        showNotification("Failed to fetch data from SMAX");

        nid_.hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ERROR));
        Shell_NotifyIcon(NIM_MODIFY, &nid_);
    }
}

void Checker::showNotification(const std::string& message) {
    nid_.uFlags |= NIF_INFO;
    std::wstring wMessage(message.begin(), message.end());
    wcscpy_s(nid_.szInfo, wMessage.c_str());
    wcscpy_s(nid_.szInfoTitle, L"Notification");
    nid_.dwInfoFlags = NIIF_INFO;
    Shell_NotifyIcon(NIM_MODIFY, &nid_);
}

} // namespace smax