// Checker.cpp
#include "Checker.h"
#include "SimpleIni.h"
#include "Requestor.h"
#include "..\resource.h"

#include <boost/json.hpp>
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

namespace json = boost::json;
namespace smax {

Checker* Checker::instance_ = nullptr;

Checker& Checker::getInstance() {
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

    worker_ = std::thread([this]() {
        while (running_) {
            std::string encoded_filter = urlEncode(filter_);
            std::string smax_url = url_ + "&filter=" + encoded_filter;
            sendGET(smax_url);
            std::this_thread::sleep_for(std::chrono::seconds(period_));
        }
    });
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

void Checker::acknowledge() {
    nid_.hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_INIT));
    Shell_NotifyIcon(NIM_MODIFY, &nid_);
}

LRESULT CALLBACK Checker::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    std::wstring wURL;

    switch (uMsg) {
        case WM_USER + 1:
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 2, L"Acknowledge");
                AppendMenu(hMenu, MF_STRING, 1, L"Shut Down");

                SetForegroundWindow(hwnd);
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);

                if (cmd == 1) {
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                } else if (cmd == 2) {
                    wURL = std::wstring(Checker::getInstance().portalURL_.begin(), Checker::getInstance().portalURL_.end());
                    ShellExecute(hwnd, L"open", wURL.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                    Checker::getInstance().acknowledge();
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
    CSimpleIniW ini;
    ini.LoadFile(iniFile_.c_str());

    period_ = std::stoi(ini.GetValue(L"Settings", L"period", L"60"));
    auto instance = ini.GetValue(L"Settings", L"instance", L"");
    
    auto ini_hostname = to_utf8(ini.GetValue(instance, L"hostname", L""));
    auto ini_tenantId = to_utf8(ini.GetValue(instance, L"tenantId", L""));
    filter_ = to_utf8(ini.GetValue(instance, L"filter", L""));
    userName_ = to_utf8(ini.GetValue(instance, L"userName", L""));
    token_ = to_utf8(ini.GetValue(instance, L"token", L""));

    url_ = "https://" + ini_hostname + "/rest/" + ini_tenantId + "/ems/Request?layout=Id";
    portalURL_ = "https://" + ini_hostname + "/saw/Requests?TENANTID=" + ini_tenantId;    
}

std::string Checker::to_utf8(const wchar_t* wstr) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr);
    return result;
}

std::vector<std::string> Checker::extract_ids_from_json(const std::string& json_str) {
    std::vector<std::string> ids;

    try {
        json::value jv = json::parse(json_str);
        json::object root = jv.as_object();

        if (root.contains("entities") && root["entities"].is_array()) {
            const json::array& entities = root["entities"].as_array();

            for (const auto& entity_val : entities) {
                if (!entity_val.is_object()) continue;
                const json::object& entity = entity_val.as_object();

                if (entity.contains("properties") && entity.at("properties").is_object()) {
                    const json::object& props = entity.at("properties").as_object();

                    if (props.contains("Id")) {
                        const auto& id_val = props.at("Id");
                        if (id_val.is_string()) {
                            ids.push_back(id_val.as_string().c_str());
                        } else if (id_val.is_int64() || id_val.is_uint64() || id_val.is_double()) {
                            ids.push_back(json::serialize(id_val));
                        }
                    }
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error parsing JSON: " << ex.what() << std::endl;
    }

    return ids;
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
    auto result = Requestor::get(url, userName_, token_);
    
    if (result.has_value()) {
        auto ids = extract_ids_from_json(result.value());
        auto newElements = update_processed_ids(ids);

        if (newElements > 0) {
            showNotification("New " + std::to_string(newElements) + " requests found!");
            nid_.hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ALERT));
            Shell_NotifyIcon(NIM_MODIFY, &nid_);
        }
    } else {
        showNotification("Failed to fetch data from SMAX");
        shutdown();
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