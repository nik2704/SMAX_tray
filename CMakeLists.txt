cmake_minimum_required(VERSION 3.12)

# Set version components
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(smax_tray_client VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# ✅ Statically link the C++ runtime if using MSVC
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    # ✅ Statically link C++ runtime and threading lib if using MinGW/GCC
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++ -lpthread")
endif()


add_definitions(-DUNICODE -D_UNICODE)

# CMake policies
cmake_policy(SET CMP0144 NEW)  # New behavior for CMAKE_INSTALL_INCLUDEDIR etc.
cmake_policy(SET CMP0167 OLD)  # Disable transitive LTO

file(GLOB IMGUI_SRC
    AviatorClient/libs/imgui/*.cpp
    AviatorClient/libs/imgui/backends/imgui_impl_win32.cpp
    AviatorClient/libs/imgui/backends/imgui_impl_opengl3.cpp
)

# Executable and source files
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

# Compiler and linker settings
target_compile_definitions(smax_tray_client PRIVATE)

# target_include_directories(smax_tray_client PRIVATE ${Boost_INCLUDE_DIRS})
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

# target_link_libraries(smax_tray_client PRIVATE crypt32 Shlwapi.lib)
# target_link_libraries(smax_tray_client PRIVATE opengl32 user32 gdi32 crypt32 Shlwapi wininet)
target_link_libraries(smax_tray_client PRIVATE opengl32 user32 gdi32 crypt32 Shlwapi wininet dwmapi)


set_target_properties(smax_tray_client PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# ✅ Define install location
install(TARGETS smax_tray_client DESTINATION bin)

#✅ NSIS Packaging (CPack)
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
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico") # Optional
set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SMAX Tray Client")

set(CPACK_CREATE_DESKTOP_LINKS "smax_tray_client")
set(CPACK_PACKAGE_EXECUTABLES "smax_tray_client" "SMAX Tray Client")

include(CPack)
