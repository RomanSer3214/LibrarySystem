cmake_minimum_required(VERSION 3.15)
project(LibrarySystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------
# Джерела
# -------------------------
file(GLOB_RECURSE SOURCES
    src/*.cpp
    libs/imgui/*.cpp
    libs/imgui/backends/*.cpp
    libs/sqlite3/sqlite3.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    libs/imgui
    libs/imgui/backends
    libs/sqlite3
)

find_package(OpenGL REQUIRED)
find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenGL::GL OpenSSL::Crypto)

if(WIN32)
    message(STATUS "Using local GLFW for Windows")
    target_include_directories(${PROJECT_NAME} PRIVATE libs/glfw/include)
    target_link_libraries(${PROJECT_NAME} 
        ${CMAKE_SOURCE_DIR}/libs/glfw_win/glfw3.lib
        opengl32
    )
elseif(UNIX AND NOT APPLE)
    message(STATUS "Using system GLFW for Linux")
    find_package(glfw3 REQUIRED)
    target_link_libraries(${PROJECT_NAME} glfw)
endif()

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES} Threads::Threads dl m)
endif()


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/fonts" "${CMAKE_BINARY_DIR}/fonts"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/data"
    COMMENT "Setting up fonts and data directories in build folder"
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

