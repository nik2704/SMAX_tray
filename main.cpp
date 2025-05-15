/// @file main.cpp
/// @brief Entry point for the SMAX checker application.
///
/// Initializes the smax::Checker singleton using configuration from `config.ini`
/// and starts the Windows message loop. On exit, the checker is properly shut down.

#include "Checker/Checker.h"
#include "TokenInitializer/TokenInitializer.h"

/**
 * @brief Windows application entry point.
 *
 * Initializes the checker with the provided instance handle and enters the message loop.
 * The checker is responsible for background processing and notifications.
 *
 * @param hInstance Handle to the current instance of the application.
 * @param hPrevInstance Unused (legacy, always NULL).
 * @param lpCmdLine The command line for the application, excluding the program name.
 * @param nCmdShow Controls how the window is to be shown (unused here).
 * @return Application exit code.
 */
INT_PTR CALLBACK InputDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static wchar_t* outBuffer = nullptr;
    switch (msg) {
        case WM_INITDIALOG:
            outBuffer = reinterpret_cast<wchar_t*>(lParam);
            SetDlgItemTextW(hwndDlg, 1001, L"");
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    GetDlgItemTextW(hwndDlg, 1001, outBuffer, 256);
                    EndDialog(hwndDlg, IDOK);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
wchar_t buffer[256] = {};
    // INT_PTR res = DialogBoxParamW(hInstance, MAKEINTRESOURCE(101), NULL, InputDlgProc, reinterpret_cast<LPARAM>(buffer));
    // if (res == IDOK) {
    //     MessageBoxW(NULL, buffer, L"Input Received", MB_OK);
    // } else {
    //     MessageBoxW(NULL, L"Dialog cancelled or failed", L"Info", MB_OK);
    // }

    // Encrypts token if it has the value "-init-"
    smax::TokenInitializer::initializeToken(L"config.ini");

    // Start the checker with the config file
    smax::Checker::getInstance().start(hInstance, L"config.ini");

    MSG msg = {};

    // Main Windows message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up on exit
    smax::Checker::getInstance().shutdown();
    return 0;
}
