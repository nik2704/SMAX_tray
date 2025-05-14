/// @file main.cpp
/// @brief Entry point for the SMAX checker application.
///
/// Initializes the smax::Checker singleton using configuration from `config.ini`
/// and starts the Windows message loop. On exit, the checker is properly shut down.

#include "Checker/Checker.h"

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
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
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
