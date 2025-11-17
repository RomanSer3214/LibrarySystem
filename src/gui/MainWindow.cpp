#include "MainWindow.h"
#include "imgui.h"

MainWindow::MainWindow() 
    : bookManager(std::make_unique<BookManager>(dbManager))
    , memberManager(std::make_unique<MemberManager>(dbManager))
    , loanManager(std::make_unique<LoanManager>(dbManager)) {}

void MainWindow::render() {
    renderMainContent();
}

void MainWindow::renderMainContent() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
                                     
    ImGui::Begin("Library Manager", nullptr, 
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
            ImGui::Text("Library Manager v0.8");
            ImGui::Text("Розроблено для курсової роботи");
            ImGui::Text("Використані технології: C++, Dear ImGui, GLFW, SQLite3");
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}
