#include <iostream>
#include "Application.h"
#include "LoginWindow.h"

Application::Application() : dbManager("data/library.db"), window(nullptr), clearColor(ImVec4(0.95f, 0.95f, 0.95f, 1.0f)) {}

Application::~Application() {
    shutdown();
}

void ImGuiTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
   
    style.TabRounding = 0.0f;        
    style.FrameRounding = 0.0f;
    style.FramePadding.x = 8.0f;
    style.FramePadding.y = 4.0f;

    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4(0.85f,0.85f,0.85f,1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.8f,0.8f,0.8f,1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.9f,0.9f,0.9f,1.0f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.1f,0.1f,0.1f,1.0f);

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.8f,0.8f,0.8f,1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.7f,0.7f,0.7f,1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.6f,0.6f,0.6f,1.0f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.85f,0.85f,0.85f,1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.75f,0.75f,0.75f,1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.65f,0.65f,0.65f,1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.9f,0.9f,0.9f,1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.8f,0.8f,0.8f,1.0f);

    // Frame
    colors[ImGuiCol_Border] = ImVec4(0.7f,0.7f,0.7f,1.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.9f,0.9f,0.9f,1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.9f,0.9f,0.9f,1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.8f,0.8f,0.8f,1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.7f,0.7f,0.7f,1.0f);

    // Menu bar
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.85f,0.85f,0.85f,1.0f);

    // Header (CollapsingHeader, TreeNode)
    colors[ImGuiCol_Header] = ImVec4(0.85f,0.85f,0.85f,1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.65f,0.65f,0.65f,1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.60f,0.60f,0.60f,1.0f);
    
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.75f,0.75f,0.75f,1.0f);
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
    
    window = glfwCreateWindow(1280, 720, "Library Manager", nullptr, nullptr);
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

    ImGuiTheme();

    io.Fonts->AddFontFromFileTTF(
        "fonts/Roboto-Regular.ttf",         // TTF
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

    static UserSession session; 

    if (!session.isLoggedIn) {
        renderLoginWindow(dbManager, session);  
    } else {
        mainWindow.render();                      
    }
    
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
