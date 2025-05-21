cmake_minimum_required(VERSION 3.12)

# Project versioning
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})
project(smax_tray_client VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# MSVC Runtime: dynamically link C++ runtime (runtime-link=shared)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

# Override in Debug if needed
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

add_definitions(-DUNICODE -D_UNICODE)

# CMake policies
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 OLD)

# Boost (vcpkg will handle it)
# Equivalent to: --with-json link=static
# Boost_USE_STATIC_LIBS ensures link=static
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF) # runtime-link=shared
find_package(Boost REQUIRED COMPONENTS json)

# Executable and source files
add_executable(smax_tray_client WIN32
    main.cpp
    TokenInitializer/TokenInitializer.cpp
    TokenInitializer/TokenDialog.rc    
    Checker/Checker.cpp
    Checker/Requestor.cpp
    Utils/Utils.cpp
    resources.rc
)

# Compile and include paths
target_compile_definitions(smax_tray_client PRIVATE BOOST_ALL_NO_LIB)
target_include_directories(smax_tray_client PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker
    ${CMAKE_CURRENT_SOURCE_DIR}/TokenInitializer
    ${CMAKE_CURRENT_SOURCE_DIR}/SimpleIni
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
)

# Linking
target_link_libraries(smax_tray_client PRIVATE
    Boost::json
    crypt32
    Shlwapi.lib
)

set_target_properties(smax_tray_client PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Installation
install(TARGETS smax_tray_client DESTINATION bin)

# Optional: Copy Boost DLLs if using shared linking (safe fallback)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/bin/"
        DESTINATION bin
        FILES_MATCHING PATTERN "*.dll")

# NSIS packaging
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
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SMAX Tray Client")
set(CPACK_CREATE_DESKTOP_LINKS "smax_tray_client")
set(CPACK_PACKAGE_EXECUTABLES "smax_tray_client" "SMAX Tray Client")

include(CPack)
