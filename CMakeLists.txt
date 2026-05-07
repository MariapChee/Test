cmake_minimum_required(VERSION 3.16)
project(CrynosExecutor VERSION 2.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)
find_package(OpenGL REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

# ImGui sources
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/include/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
)

# Application sources
set(APP_SOURCES
    src/main.cpp
    src/app.cpp
    src/editor.cpp
    src/console.cpp
    src/script_hub.cpp
    src/saved_scripts.cpp
    src/settings.cpp
    src/ai_chat.cpp
    src/http_client.cpp
    src/theme.cpp
    src/utils.cpp
    src/stb_impl.cpp
)

add_executable(${PROJECT_NAME} ${APP_SOURCES} ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/imgui
    ${CMAKE_SOURCE_DIR}/include/json
    ${CMAKE_SOURCE_DIR}/include/stb
    ${GLFW3_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GLFW3_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${CURL_LIBRARIES}
    Threads::Threads
    dl
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GL3W
    CRYNOS_VERSION="${PROJECT_VERSION}"
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
