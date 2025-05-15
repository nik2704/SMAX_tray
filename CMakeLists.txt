cmake_minimum_required(VERSION 3.12)

# Set version components
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(smax_tray_client VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_definitions(-DUNICODE -D_UNICODE)


# Boost configuration
set(BOOST_ROOT "C:/boost/boost_1_87_0")
set(Boost_NO_SYSTEM_PATHS ON)

# CMake policies
cmake_policy(SET CMP0144 NEW)  # New behavior for CMAKE_INSTALL_INCLUDEDIR etc.
cmake_policy(SET CMP0167 OLD)  # Disable transitive LTO

# Find Boost
find_package(Boost 1.87 REQUIRED COMPONENTS json)

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

# Compiler and linker settings
target_compile_definitions(smax_tray_client PRIVATE BOOST_ALL_NO_LIB)

# target_include_directories(smax_tray_client PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(smax_tray_client PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Checker
    ${CMAKE_CURRENT_SOURCE_DIR}/TokenInitializer
    ${CMAKE_CURRENT_SOURCE_DIR}/SimpleIni
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
)

target_link_libraries(smax_tray_client PRIVATE ${Boost_LIBRARIES} crypt32 Boost::json Shlwapi.lib)

set_target_properties(smax_tray_client PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Packaging (CPack)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
