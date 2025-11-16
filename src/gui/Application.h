#pragma once
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "MainWindow.h"
#include "../database/DatabaseManager.h"

class Application {
private:
    DatabaseManager dbManager;
    GLFWwindow* window;
    MainWindow mainWindow;
    ImVec4 clearColor;

    bool setupWindow();
    bool setupImGui();
    void render();
    void processInput();

public:
    Application();
    ~Application();
    
    bool initialize();
    void run();
    void shutdown();
};
