#pragma once
#include <GLFW/glfw3.h>
#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/backends/imgui_impl_glfw.h"
#include "../libs/imgui/backends/imgui_impl_opengl3.h"
#include "MainWindow.h"
#include "../database/DatabaseManager.h"

class Application {
private:
    GLFWwindow* window;
    MainWindow mainWindow;
    DatabaseManager dbManager;
    ImVec4 clearColor;

public:
    Application();
    ~Application();
    
    bool initialize();
    void run();
    void shutdown();

private:
    bool setupWindow();
    bool setupImGui();
    void render();
    void processInput();
};
