#include "LoanManager.h"

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

void LoanManager::addLoan() {
    if (selectedBookIndex < 0 || selectedMemberIndex < 0) return;

    Book& book = books[selectedBookIndex];
    Member& member = members[selectedMemberIndex];

    if (!book.isAvailable()) return;

    // Створюємо об'єкт Loan
    int newId = static_cast<int>(loans.size()) + 1; 
    Loan newLoan(newId, book.getISBN(), member.getId(), loanDays);

    loans.push_back(newLoan);

    book.borrowBook();

    selectedBookIndex = -1;
    selectedMemberIndex = -1;

    dbManager.addLoan(newLoan);

    ImGui::OpenPopup("Позичка успішна");
}


// --- ПОЗИЧАННЯ КНИГИ ---
void LoanManager::renderBorrowSection() {
    ImGui::Text("Пошук книги:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(400);  
    ImGui::InputText("##bookSearch", searchBuffer, sizeof(searchBuffer));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(96);
    ImGui::InputInt("Термін позичення (днів)", &loanDays);

    const int MIN_DAYS = 1;
    const int MAX_DAYS = 60;

    if (loanDays < MIN_DAYS) loanDays = MIN_DAYS;
    if (loanDays > MAX_DAYS) loanDays = MAX_DAYS;

    if (ImGui::Button("Позичити книгу")) {
        if (selectedBookIndex >= 0 && selectedMemberIndex >= 0) {
            addLoan();
        } else {
            ImGui::OpenPopup("Помилка");
        }
    }

    if (ImGui::BeginPopupModal("Помилка", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Будь ласка, оберіть книгу та читача!");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Columns(2, nullptr, false);

    if (ImGui::BeginChild("BooksTableChild", ImVec2(0, 400), true)) {
        if (ImGui::BeginTable("BooksTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("ISBN");
            ImGui::TableSetupColumn("Назва");
            ImGui::TableSetupColumn("Автор");
            ImGui::TableSetupColumn("Доступно");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < books.size(); i++) {
                const auto& book = books[i];

                if (strlen(searchBuffer) > 0) {
                    if (book.getTitle().find(searchBuffer) == std::string::npos &&
                        book.getAuthor().find(searchBuffer) == std::string::npos &&
                        book.getISBN().find(searchBuffer) == std::string::npos) {
                        continue;
                    }
                }

                if (!book.isAvailable()) continue;

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(book.getISBN().c_str(), selectedBookIndex == static_cast<int>(i), ImGuiSelectableFlags_SpanAllColumns)) {
                    selectedBookIndex = i;
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", book.getTitle().c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", book.getAuthor().c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%d", book.getAvailableCopies());
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();
    }

    ImGui::NextColumn(); // права колонка — читачі

    if (ImGui::BeginChild("MembersTableChild", ImVec2(0, 400), true)) {
        if (ImGui::BeginTable("MembersTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Ім'я");
            ImGui::TableSetupColumn("Телефон");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < members.size(); i++) {
                const auto& member = members[i];

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(std::to_string(member.getId()).c_str(), selectedMemberIndex == static_cast<int>(i), ImGuiSelectableFlags_SpanAllColumns)) {
                    selectedMemberIndex = i;
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", member.getName().c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", member.getPhone().c_str());
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();
    }

    ImGui::Columns(1); // скидаємо колонки назад
}
// --- ПОВЕРНЕННЯ КНИГИ ---
void LoanManager::renderReturnSection() {
    ImGui::Text("Повернення книги");
    ImGui::Separator();

    std::vector<Loan> activeLoans = dbManager.getActiveLoans();
    if (activeLoans.empty()) {
        ImGui::Text("Немає активних позичень");
        return;
    }

    if (ImGui::BeginTable("ActiveLoans", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Книга");
        ImGui::TableSetupColumn("Читач");
        ImGui::TableSetupColumn("Дата позичення");
        ImGui::TableSetupColumn("Термін повернення");
        ImGui::TableSetupColumn("Дія");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < activeLoans.size(); ++i) {
            Loan& loan = activeLoans[i];
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", getBookTitle(loan.getBookISBN()).c_str());
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", getMemberName(loan.getMemberId()).c_str());
            ImGui::TableSetColumnIndex(2); ImGui::Text("-");
            ImGui::TableSetColumnIndex(3); ImGui::Text("-");

            ImGui::TableSetColumnIndex(4);
            if (ImGui::Button(("Повернути##" + std::to_string(i)).c_str())) {
                if (returnLoan(loan)) {
                    activeLoans.erase(activeLoans.begin() + i);
                    --i;
                    ImGui::OpenPopup("Повернено");
                }
            }
        }
        ImGui::EndTable();
    }

    if (ImGui::BeginPopupModal("Повернено", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Книга успішно повернена!");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

bool LoanManager::returnLoan(Loan& loan) {
    Book* book = dbManager.findBook(loan.getBookISBN());
    if (!book) return false;

    if (!book->returnBook()) return false;
    dbManager.updateBook(*book);

    loan.setIsReturned(true);
    loan.setReturnDate(std::chrono::system_clock::now());
    return dbManager.updateLoan(loan);
}

// --- ВІДОБРАЖЕННЯ ВСІХ ПОЗИЧОК ---
void LoanManager::renderLoanList() {
    ImGui::Text("Усі позички:");
    ImGui::BeginTable("loanTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
    ImGui::Text("ID"); ImGui::NextColumn();
    ImGui::Text("Book ISBN"); ImGui::NextColumn();
    ImGui::Text("Member ID"); ImGui::NextColumn();
    ImGui::Text("Loan Date"); ImGui::NextColumn();
    ImGui::Text("Return Date"); ImGui::NextColumn();
    ImGui::Separator();

    auto toString = [](std::chrono::system_clock::time_point tp) -> std::string {
        if (tp.time_since_epoch().count() == 0) return "-";
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        std::tm tm;
    #ifdef _WIN32
        localtime_s(&tm, &t);
    #else
        localtime_r(&t, &tm);
    #endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
        return std::string(buf);
    };

    for (size_t i = 0; i < loans.size(); ++i) {
        const Loan& loan = loans[i];
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", loan.getId());
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", loan.getBookISBN().c_str());
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%d", loan.getMemberId());
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%s", toString(loan.getLoanDate()).c_str());
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%s", toString(loan.getReturnDate()).c_str());
    }
    ImGui::EndTable();
}

void LoanManager::loadData() {
    books = dbManager.getAllBooks();
    members = dbManager.getAllMembers();
    loans = dbManager.getAllLoans();
}

std::string LoanManager::getBookTitle(const std::string& isbn) const {
    for (const auto& book : books)
        if (book.getISBN() == isbn) return book.getTitle();
    return "Невідома книга";
}

std::string LoanManager::getMemberName(int memberId) const {
    for (const auto& member : members)
        if (member.getId() == memberId) return member.getName();
    return "Невідомий читач";
}

