#include "TrayManager.h"
#include "../resource.h"
#include <shellapi.h>

namespace smax {

TrayManager* TrayManager::instance_ = nullptr;

TrayManager::TrayManager() : hwnd_(nullptr), hInstance_(nullptr) {
    instance_ = this;
    memset(&nid_, 0, sizeof(nid_));
}

TrayManager::~TrayManager() {
    shutdown();
    instance_ = nullptr;
}

void TrayManager::initialize(HINSTANCE hInst) {
    hInstance_ = hInst;
    const wchar_t CLASS_NAME[] = L"SMAXTrayAppClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = TrayManager::WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    hwnd_ = CreateWindowEx(0, CLASS_NAME, L"SMAXTrayWindow", 0,
                           0, 0, 0, 0, nullptr, nullptr, hInst, nullptr);

    createTrayIcon();
}

void TrayManager::createTrayIcon() {
    nid_.cbSize = sizeof(nid_);
    nid_.hWnd = hwnd_;
    nid_.uID = 1;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_USER + 1;
    nid_.hIcon = LoadIcon(hInstance_, MAKEINTRESOURCE(SMAX_TRAY_ICON_INIT));
    wcscpy_s(nid_.szTip, L"SMAX requests");

    Shell_NotifyIcon(NIM_ADD, &nid_);
}

void TrayManager::destroyTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid_);
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void TrayManager::shutdown() {
    destroyTrayIcon();
}

void TrayManager::setIcon(HICON hIcon) {
    nid_.hIcon = hIcon;
    Shell_NotifyIcon(NIM_MODIFY, &nid_);
}

void TrayManager::showInfo(const std::wstring& message, const std::wstring& title) {
    nid_.uFlags |= NIF_INFO;
    wcscpy_s(nid_.szInfo, message.c_str());
    wcscpy_s(nid_.szInfoTitle, title.c_str());
    nid_.dwInfoFlags = NIIF_INFO;
    Shell_NotifyIcon(NIM_MODIFY, &nid_);
}

void TrayManager::showErrorMessage(const std::wstring& title, const std::wstring& message) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd_;
    nid.uFlags = NIF_INFO;
    wcsncpy_s(nid.szInfo, message.c_str(), _TRUNCATE);
    wcsncpy_s(nid.szInfoTitle, title.c_str(), _TRUNCATE);
    nid.dwInfoFlags = NIIF_ERROR;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void TrayManager::dismissAlert() {
    wcscpy_s(nid_.szInfo, L"");
    wcscpy_s(nid_.szInfoTitle, L"");
    nid_.dwInfoFlags = NIIF_NONE;
    Shell_NotifyIcon(NIM_MODIFY, &nid_);
}

void TrayManager::setOnAcknowledgeInbox(std::function<void()> callback) {
    onAcknowledgeInbox_ = std::move(callback);
}

void TrayManager::setOnAcknowledgeRequests(std::function<void()> callback) {
    onAcknowledgeRequests_ = std::move(callback);
}
void TrayManager::setOnAcknowledgeTasks(std::function<void()> callback) {
    onAcknowledgeTasks_ = std::move(callback);
}
void TrayManager::setOnAcknowledgeApprovals(std::function<void()> callback) {
    onAcknowledgeApprovals_ = std::move(callback);
}

void TrayManager::setOnShutdown(std::function<void()> callback) {
    onShutdown_ = std::move(callback);
}

void TrayManager::setOnUpdateConfig(std::function<void()> callback) {
    onUpdateConfig_ = std::move(callback);
}

void TrayManager::openURLinBrowser(const std::wstring& url) {
    ShellExecute(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

LRESULT CALLBACK TrayManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_USER + 1) {
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, L"Shut Down");
            // AppendMenu(hMenu, MF_STRING, 2, L"Acknowledge");

            HMENU hAcknowledgeMenu = CreatePopupMenu();
            AppendMenu(hAcknowledgeMenu, MF_STRING, 1001, L"Inbox");

            if (instance_->requests_ > 0)
                AppendMenu(hAcknowledgeMenu, MF_STRING, 1002, L"Requests");
            if (instance_->tasks_ > 0)
                AppendMenu(hAcknowledgeMenu, MF_STRING, 1003, L"Tasks");
            if (instance_->approvals_ > 0)
                AppendMenu(hAcknowledgeMenu, MF_STRING, 1004, L"Approvals");

            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hAcknowledgeMenu, L"Acknowledge");

            AppendMenu(hMenu, MF_STRING, 3, L"Settings");

            SetForegroundWindow(hwnd);
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);

            if (cmd == 1 && instance_->onShutdown_) instance_->onShutdown_();
            // else if (cmd == 2 && instance_->onAcknowledge_) instance_->onAcknowledge_();
            else if (cmd == 1001 && instance_->onAcknowledgeInbox_) {
                instance_->onAcknowledgeInbox_();
            } else if (cmd == 1002 && instance_->onAcknowledgeRequests_) {
                instance_->onAcknowledgeRequests_();
            } else if (cmd == 1003 && instance_->onAcknowledgeTasks_) {
                instance_->onAcknowledgeTasks_();
            } else if (cmd == 1004 && instance_->onAcknowledgeApprovals_) {
                instance_->onAcknowledgeApprovals_();
            }  else if (cmd == 3 && instance_->onUpdateConfig_) instance_->onUpdateConfig_();


        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void TrayManager::acknowledgeInbox() {
    if (onAcknowledgeInbox_) {
        onAcknowledgeInbox_();
    }

    dismissAlert();
}

void TrayManager::acknowledgeRequests() {
    if (onAcknowledgeRequests_) {
        onAcknowledgeRequests_();
    }

    dismissAlert();
}
void TrayManager::acknowledgeTasks() {
    if (onAcknowledgeTasks_) {
        onAcknowledgeTasks_();
    }

    dismissAlert();
}
void TrayManager::acknowledgeApprovals() {
    if (onAcknowledgeApprovals_) {
        onAcknowledgeApprovals_();
    }

    dismissAlert();
}

void TrayManager::updateTooltip(int requests, int tasks, int approvals) {
    std::wstring tooltip;

    requests_ = requests;
    tasks_ = tasks;
    approvals_ = approvals;

    if (requests == 0 && tasks == 0 && approvals == 0) {
        tooltip = L"SMAX tray client";
    } else {
        if (requests > 0) {
            tooltip += std::to_wstring(requests) + L" new request(s)";
        }

        if (tasks > 0) {
            if (!tooltip.empty()) tooltip += L" + ";
            tooltip += std::to_wstring(tasks) + L" new task(s)";
        }

        if (approvals > 0) {
            if (!tooltip.empty()) tooltip += L" + ";
            tooltip += std::to_wstring(approvals) + L" new approval(s)";
        }
    }

    wcsncpy_s(nid_.szTip, tooltip.c_str(), _TRUNCATE);
    nid_.uFlags |= NIF_TIP;
    Shell_NotifyIcon(NIM_MODIFY, &nid_);
}


} // namespace smax
