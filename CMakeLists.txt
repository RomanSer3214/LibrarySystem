cmake_minimum_required(VERSION 3.15)
project(LibrarySystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Використовуємо системний GLFW
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# Всі файли проекту
file(GLOB_RECURSE SOURCES
    src/*.cpp
    libs/imgui/*.cpp
    libs/imgui/backends/*.cpp
    libs/sqlite3/sqlite3.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Інклюди
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    libs/imgui
    libs/imgui/backends
    libs/sqlite3
)

# Лінковка
target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL)

# Linux лінковка
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES} Threads::Threads dl m)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts
        ${CMAKE_BINARY_DIR}/fonts
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying fonts and data directories to build folder"
)

# Опції компіляції
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
