#include "MainWindow.h"
#include "imgui.h"

MainWindow::MainWindow() 
    : bookManager(std::make_unique<BookManager>(dbManager))
    , memberManager(std::make_unique<MemberManager>(dbManager))
    , loanManager(std::make_unique<LoanManager>(dbManager)) {}

void MainWindow::render() {
    renderMainContent();
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

void MainWindow::renderMainContent() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGuiTheme(); 
                                     
    ImGui::Begin("Бібліотечна система", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Книги")) {
            bookManager->render();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Читачі")) {
            memberManager->render();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Позичення")) {
            loanManager->render();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Про систему")) {
            ImGui::Text("Бібліотечна система v1.0");
            ImGui::Text("Розроблено для курсової роботи");
            ImGui::Text("Використані технології: C++, Dear ImGui, GLFW, SQLite3");
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}
