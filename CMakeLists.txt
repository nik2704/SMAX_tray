cmake_minimum_required(VERSION 3.12)

# Set version components
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(smax_tray_client VERSION ${PROJECT_VERSION} LANGUAGES CXX)


# ✅ Statically link the C++ runtime if using MSVC
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


add_definitions(-DUNICODE -D_UNICODE)

# CMake policies
cmake_policy(SET CMP0144 NEW)  # New behavior for CMAKE_INSTALL_INCLUDEDIR etc.
cmake_policy(SET CMP0167 OLD)  # Disable transitive LTO

# Executable and source files
add_executable(smax_tray_client WIN32
    main.cpp
    TokenInitializer/TokenInitializer.cpp
    TokenInitializer/TokenDialog.rc    
    Checker/Checker.cpp

    Checker/JSONhelper/json_builder.cpp
    Checker/JSONhelper/json.cpp
    Checker/JSONhelper/JSONhelper.cpp
    Checker/NetworkClient/NetworkClient.cpp
    Checker/ConfigManager/ConfigManager.cpp
    Checker/TrayManager/TrayManager.cpp
    Checker/Worker/Worker.cpp
    Utils/Utils.cpp
    resources.rc
)

# Compiler and linker settings
target_compile_definitions(smax_tray_client PRIVATE)

# target_include_directories(smax_tray_client PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(smax_tray_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/JSONhelper
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/NetworkClient
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/ConfigManager
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker/TrayManager
    ${CMAKE_CURRENT_SOURCE_DIR}/TokenInitializer
    ${CMAKE_CURRENT_SOURCE_DIR}/Worker
    ${CMAKE_CURRENT_SOURCE_DIR}/SimpleIni
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
)

target_link_libraries(smax_tray_client PRIVATE crypt32 Shlwapi.lib)

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
