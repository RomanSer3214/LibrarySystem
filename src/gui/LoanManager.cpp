#include "LoanManager.h"
#include <algorithm>

LoanManager::LoanManager(DatabaseManager& db) : dbManager(db) {
    loadData();
}

void LoanManager::render() {
    if (ImGui::BeginTabBar("LoanTabs")) {
        if (ImGui::BeginTabItem("Позичити книгу")) {
            renderBorrowSection();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Повернути книгу")) {
            renderReturnSection();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Активні позичення")) {
            renderLoanList();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}

void LoanManager::renderBorrowSection() {
    ImGui::Text("Позичення книги");
    ImGui::Separator();
    
    // Пошук книги
    ImGui::InputText("Пошук книги", bookSearchBuffer, sizeof(bookSearchBuffer));
    
    if (ImGui::BeginTable("AvailableBooks", 4, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_ScrollY)) {
        
        ImGui::TableSetupColumn("ISBN");
        ImGui::TableSetupColumn("Назва");
        ImGui::TableSetupColumn("Автор");
        ImGui::TableSetupColumn("Доступно");
        ImGui::TableHeadersRow();
        
        for (int i = 0; i < books.size(); i++) {
            const auto& book = books[i];
            
            // Фільтрація за пошуком
            if (strlen(bookSearchBuffer) > 0) {
                if (book.getTitle().find(bookSearchBuffer) == std::string::npos &&
                    book.getAuthor().find(bookSearchBuffer) == std::string::npos &&
                    book.getISBN().find(bookSearchBuffer) == std::string::npos) {
                    continue;
                }
            }
            
            // Показувати тільки доступні книги
            if (!book.isAvailable()) {
                continue;
            }
            
            ImGui::TableNextRow();
            
            if (i == selectedBookIndex) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 
                    ImGui::GetColorU32(ImGuiCol_Header));
            }
            
            if (ImGui::IsItemClicked()) {
                selectedBookIndex = i;
            }
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", book.getISBN().c_str());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", book.getTitle().c_str());
            
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", book.getAuthor().c_str());
            
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", book.getAvailableCopies());
        }
        
        ImGui::EndTable();
    }
    
    // Пошук читача
    ImGui::Spacing();
    ImGui::InputText("Пошук читача", memberSearchBuffer, sizeof(memberSearchBuffer));
    
    if (ImGui::BeginTable("AvailableMembers", 3, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_ScrollY)) {
        
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Ім'я");
        ImGui::TableSetupColumn("Тип");
        ImGui::TableHeadersRow();
        
        for (int i = 0; i < members.size(); i++) {
            const auto& member = members[i];
            
            // Фільтрація за пошуком
            if (strlen(memberSearchBuffer) > 0) {
                if (member.getName().find(memberSearchBuffer) == std::string::npos &&
                    member.getEmail().find(memberSearchBuffer) == std::string::npos) {
                    continue;
                }
            }
            
            ImGui::TableNextRow();
            
            if (i == selectedMemberIndex) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 
                    ImGui::GetColorU32(ImGuiCol_Header));
            }
            
            if (ImGui::IsItemClicked()) {
                selectedMemberIndex = i;
            }
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", member.getId());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", member.getName().c_str());
            
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", member.getTypeString().c_str());
        }
        
        ImGui::EndTable();
    }
    
    // Кнопка позичення
    ImGui::Spacing();
    ImGui::InputInt("Термін позичення (днів)", &loanDays);
    
    if (ImGui::Button("Позичити книгу", ImVec2(200, 40))) {
        if (selectedBookIndex >= 0 && selectedMemberIndex >= 0) {
            borrowBook();
        } else {
            ImGui::OpenPopup("Помилка");
        }
    }
    
    // Попап помилки
    if (ImGui::BeginPopupModal("Помилка", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Будь ласка, оберіть книгу та читача!");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void LoanManager::renderReturnSection() {
    ImGui::Text("Повернення книги");
    ImGui::Separator();
    
    std::vector<Loan> activeLoans = dbManager.getActiveLoans();
    
    if (activeLoans.empty()) {
        ImGui::Text("Немає активних позичень");
        return;
    }
    
    if (ImGui::BeginTable("ActiveLoans", 5, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_ScrollY)) {
        
        ImGui::TableSetupColumn("Книга");
        ImGui::TableSetupColumn("Читач");
        ImGui::TableSetupColumn("Дата позичення");
        ImGui::TableSetupColumn("Термін повернення");
        ImGui::TableSetupColumn("Дія");
        ImGui::TableHeadersRow();
        
        for (int i = 0; i < activeLoans.size(); i++) {
            const auto& loan = activeLoans[i];
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", getBookTitle(loan.getBookISBN()).c_str());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", getMemberName(loan.getMemberId()).c_str());
            
            ImGui::TableSetColumnIndex(2);
            // Тут можна додати форматування дати
            
            ImGui::TableSetColumnIndex(3);
            // Тут можна додати форматування дати
            
            ImGui::TableSetColumnIndex(4);
            if (ImGui::Button("Повернути")) {
                returnBook(i);
            }
        }
        
        ImGui::EndTable();
    }
}

void LoanManager::renderLoanList() {
    if (ImGui::BeginTable("AllLoans", 6, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_ScrollY)) {
        
        ImGui::TableSetupColumn("Книга");
        ImGui::TableSetupColumn("Читач");
        ImGui::TableSetupColumn("Дата позичення");
        ImGui::TableSetupColumn("Термін повернення");
        ImGui::TableSetupColumn("Статус");
        ImGui::TableSetupColumn("Штраф");
        ImGui::TableHeadersRow();
        
        for (const auto& loan : loans) {
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", getBookTitle(loan.getBookISBN()).c_str());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", getMemberName(loan.getMemberId()).c_str());
            
            ImGui::TableSetColumnIndex(2);
            // Дата позичення
            
            ImGui::TableSetColumnIndex(3);
            // Термін повернення
            
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", loan.getIsReturned() ? "Повернено" : "Активне");
            
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%.2f грн", loan.getFineAmount());
        }
        
        ImGui::EndTable();
    }
}

void LoanManager::borrowBook() {
    if (selectedBookIndex >= 0 && selectedMemberIndex >= 0) {
        const auto& book = books[selectedBookIndex];
        const auto& member = members[selectedMemberIndex];
        
        Loan newLoan(book.getISBN(), member.getId(), loanDays);
        if (dbManager.addLoan(newLoan)) {
            // Оновити статус книги
            loadData();
            selectedBookIndex = -1;
            selectedMemberIndex = -1;
        }
    }
}

void LoanManager::returnBook(int loanIndex) {
    std::vector<Loan> activeLoans = dbManager.getActiveLoans();
    if (loanIndex >= 0 && loanIndex < activeLoans.size()) {
        const auto& loan = activeLoans[loanIndex];
        // Логіка повернення книги
        loadData();
    }
}

void LoanManager::loadData() {
    books = dbManager.getAllBooks();
    members = dbManager.getAllMembers();
    loans = dbManager.getAllLoans();
}

std::string LoanManager::getBookTitle(const std::string& isbn) {
    for (const auto& book : books) {
        if (book.getISBN() == isbn) {
            return book.getTitle();
        }
    }
    return "Невідома книга";
}

std::string LoanManager::getMemberName(int memberId) {
    for (const auto& member : members) {
        if (member.getId() == memberId) {
            return member.getName();
        }
    }
    return "Невідомий читач";
}
