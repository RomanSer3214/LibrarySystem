cmake_minimum_required(VERSION 3.20)
project(LibrarySystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")

# Collect all source files
file(GLOB_RECURSE CORE_SOURCES
    "${SRC_DIR}/core/*.cpp"
)
file(GLOB_RECURSE DATABASE_SOURCES
    "${SRC_DIR}/database/*.cpp"
)
file(GLOB_RECURSE GUI_SOURCES
    "${SRC_DIR}/gui/*.cpp"
)
set(MAIN_SOURCE "${SRC_DIR}/main.cpp")

# ImGui sources
file(GLOB IMGUI_SOURCES
    "${LIBS_DIR}/imgui/*.cpp"
    "${LIBS_DIR}/imgui/backends/*.cpp"
)

# SHA256 sources
set(SHA256_SOURCES
    "${LIBS_DIR}/sha256/sha256.c"
)

# SQLite3 sources
set(SQLITE3_SOURCES
    "${LIBS_DIR}/sqlite3/sqlite3.c"
)

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${DATABASE_SOURCES}
    ${GUI_SOURCES}
    ${MAIN_SOURCE}
    ${IMGUI_SOURCES}
    ${SHA256_SOURCES}
    ${SQLITE3_SOURCES}
)

# Add executable
add_executable(LibrarySystem ${ALL_SOURCES})

# Include directories
target_include_directories(LibrarySystem PRIVATE
    "${SRC_DIR}"
    "${SRC_DIR}/core"
    "${SRC_DIR}/database"
    "${SRC_DIR}/gui"
    "${LIBS_DIR}/imgui"
    "${LIBS_DIR}/imgui/backends"
    "${LIBS_DIR}/sha256"
    "${LIBS_DIR}/sqlite3"
    "${LIBS_DIR}/glfw-3.4.bin.WIN32/include"
)

# Link GLFW
target_link_directories(LibrarySystem PRIVATE
    "${LIBS_DIR}/glfw-3.4.bin.WIN32/lib-vc2022"
)

target_link_libraries(LibrarySystem PRIVATE
    glfw3_mt.lib
    opengl32
)

# Copy GLFW DLL to build folder
add_custom_command(TARGET LibrarySystem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LIBS_DIR}/glfw-3.4.bin.WIN32/lib-vc2022/glfw3.dll"
    $<TARGET_FILE_DIR:LibrarySystem>

    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:LibrarySystem>/data
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:LibrarySystem>/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/fonts"
        "$<TARGET_FILE_DIR:LibrarySystem>/fonts"
)

# Optional: define macros for ImGui backends
target_compile_definitions(LibrarySystem PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)
