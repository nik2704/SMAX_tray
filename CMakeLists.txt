cmake_minimum_required(VERSION 3.12)

# Project version
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(smax_tray_client VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# ✅ Statically link the C++ runtime library depending on compiler
if(MSVC)
    # Use /MT or /MTd (Multithreaded static CRT for Release/Debug)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    # ✅ For MinGW/GCC: statically link GCC, stdlib, and pthreads
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

# ✅ Unicode definitions for Windows API
add_definitions(-DUNICODE -D_UNICODE)

# CMake policies (CMP0144: control install include dir behavior, CMP0167: disable LTO propagation)
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 OLD)

# ✅ Collect Dear ImGui source files
file(GLOB IMGUI_SRC
    AviatorClient/libs/imgui/*.cpp
    AviatorClient/libs/imgui/backends/imgui_impl_win32.cpp
    AviatorClient/libs/imgui/backends/imgui_impl_opengl3.cpp
)

# ✅ Define the executable with source files
add_executable(smax_tray_client WIN32
    main.cpp
    Checker/ConfigManager/ConfigInitializer/ConfigInitializer.cpp
    Checker/ConfigManager/ConfigInitializer/ConfigInitializerDialog.rc    
    Checker/Checker.cpp
    Checker/ConfigManager/ConfigManager.cpp
    Checker/TrayManager/TrayManager.cpp
    Checker/Worker/Worker.cpp
    libs/JSONhelper/json_builder.cpp
    libs/JSONhelper/json.cpp
    libs/JSONhelper/JSONhelper.cpp
    libs/NetworkClient/NetworkClient.cpp
    libs/Logger/Logger.cpp
    Utils/Utils.cpp
    resources.rc

    AviatorClient/src/ImGuiLayer.cpp
    AviatorClient/src/ChatUI.cpp
    AviatorClient/src/TextureLoader.cpp
    AviatorClient/src/ConversationManager.cpp
    AviatorClient/src/ChatController.cpp
    AviatorClient/src/StringJSONextractor.cpp
    AviatorClient/src/utils.cpp
    ${IMGUI_SRC}
)

# ✅ Include directories
target_include_directories(smax_tray_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/JSONhelper
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/NetworkClient
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/ConfigManager
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/ConfigManager/ConfigInitializer
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/TrayManager
    ${CMAKE_CURRENT_SOURCE_DIR}/Worker
    ${CMAKE_CURRENT_SOURCE_DIR}/SimpleIni
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/AviatorClient/assets
    ${CMAKE_CURRENT_SOURCE_DIR}/AviatorClient/libs/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/AviatorClient/libs/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/AviatorClient/libs/stb_image
    ${CMAKE_CURRENT_SOURCE_DIR}/AviatorClient/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/AviatorClient/libs/NetworkClient
    ${CMAKE_CURRENT_SOURCE_DIR}/AviatorClient/libs/JSONhelper    
)

# ✅ Link against Windows system libraries (which are expected to be dynamic)
# ❗ For MinGW, pthread must be added explicitly for static builds
target_link_libraries(smax_tray_client PRIVATE
    opengl32
    user32
    gdi32
    crypt32
    Shlwapi
    wininet
    dwmapi
    pthread  # Needed for static libstdc++ and libgcc
)

# ✅ Set C++ standard
set_target_properties(smax_tray_client PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# ✅ Installation directory for the binary
install(TARGETS smax_tray_client DESTINATION bin)

# ✅ NSIS Installer Packaging with CPack
include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR NSIS)
set(CPACK_PACKAGE_NAME "SMAX Tray Client")
set(CPACK_PACKAGE_VENDOR "OT")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SMAX Tray Utility for Windows")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "example@example.com")

set(CPACK_NSIS_DISPLAY_NAME "SMAX Tray Client")
set(CPACK_NSIS_PACKAGE_NAME "SMAX_Tray_Client")
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico") # Optional custom icon
set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SMAX Tray Client")

# Create desktop and start menu shortcuts
set(CPACK_CREATE_DESKTOP_LINKS "smax_tray_client")
set(CPACK_PACKAGE_EXECUTABLES "smax_tray_client" "SMAX Tray Client")

include(CPack)
