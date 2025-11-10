#pragma once
#include "imgui.h"
#include "BookManager.h"
#include "MemberManager.h"
#include "LoanManager.h"
#include "../database/DatabaseManager.h"
#include <memory>

class MainWindow {
private:
    std::unique_ptr<BookManager> bookManager;
    std::unique_ptr<MemberManager> memberManager;
    std::unique_ptr<LoanManager> loanManager;
    DatabaseManager dbManager;
    
    bool showDemoWindow = false;

public:
    MainWindow();
    void render();

private:
    void renderMenuBar();
    void renderMainContent();
};
