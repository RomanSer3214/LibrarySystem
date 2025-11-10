#include "Application.h"
#include <iostream>

Application::Application() : window(nullptr), clearColor(ImVec4(0.45f, 0.55f, 0.60f, 1.00f)) {}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    if (!setupWindow()) {
        return false;
    }
    
    if (!setupImGui()) {
        return false;
    }
    
    return true;
}

bool Application::setupWindow() {
    if (!glfwInit()) {
        std::cerr << "Не вдалося ініціалізувати GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    window = glfwCreateWindow(1280, 720, "Бібліотечна система", nullptr, nullptr);
    if (!window) {
        std::cerr << "Не вдалося створити вікно GLFW" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync
    
    return true;
}

bool Application::setupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    io.Fonts->AddFontFromFileTTF(
        "fonts/Roboto-Regular.ttf",         // ваш шлях до TTF
        18.0f,                              
        nullptr,
        io.Fonts->GetGlyphRangesCyrillic()  
    );

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Не вдалося ініціалізувати ImGui для GLFW" << std::endl;
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 130")) {
        std::cerr << "Не вдалося ініціалізувати ImGui для OpenGL" << std::endl;
        return false;
    }
    return true;
}

void Application::run() {
    while (!glfwWindowShouldClose(window)) {
        processInput();
        render();
    }
}

void Application::processInput() {
    glfwPollEvents();
}

void Application::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    mainWindow.render();
    
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(window);
}

void Application::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
