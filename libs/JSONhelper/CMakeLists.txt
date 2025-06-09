add_library(JSONhelper STATIC
    json_builder.cpp
    json.cpp
    JSONhelper.cpp
)

target_include_directories(JSONhelper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS JSONhelper ARCHIVE DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include FILES_MATCHING PATTERN "*.h")
