#include <chrono>
#include "LoanManager.h"
#include "imgui.h"

LoanManager::LoanManager(DatabaseManager& db) : dbManager(db) {
    loadData();
}

void LoanManager::render() {
    if (ImGui::BeginTabBar("LoanTabs")) {
        if (ImGui::BeginTabItem("Позичити книгу")) {
            // Refresh data when opening the borrow tab so we always show current books/members
            loadData();
            renderBorrowSection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Повернути книгу")) {
            // Refresh active loans/books before showing return UI
            loadData();
            renderReturnSection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Активні позичення")) {
            // Ensure the list of loans is current
            loadData();
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

    // Create Loan with placeholder id (DB will assign id via AUTOINCREMENT)
    Loan newLoan(0, book.getISBN(), member.getId(), loanDays);

    // Let DatabaseManager handle transactional changes (it will decrement available_copies)
    if (dbManager.addLoan(newLoan)) {
        // Refresh local cache after DB change
        loadData();
        selectedBookIndex = -1;
        selectedMemberIndex = -1;
        ImGui::OpenPopup("Позичка успішна");
    } else {
        ImGui::OpenPopup("Помилка"); // existing "Помилка" popup handles messaging
    }
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
                    selectedBookIndex = static_cast<int>(i);
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
                    selectedMemberIndex = static_cast<int>(i);
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

    auto toString = [](const std::chrono::system_clock::time_point& tp) -> std::string {
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

    if (ImGui::BeginTable("ActiveLoans", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Книга");
        ImGui::TableSetupColumn("Читач");
        ImGui::TableSetupColumn("Дата позичення");
        ImGui::TableSetupColumn("Термін повернення");
        ImGui::TableSetupColumn("Статус");
        ImGui::TableSetupColumn("Дія");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < activeLoans.size(); ++i) {
            Loan& loan = activeLoans[i];
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", getBookTitle(loan.getBookISBN()).c_str());
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", getMemberName(loan.getMemberId()).c_str());
            ImGui::TableSetColumnIndex(2); ImGui::Text("%s", toString(loan.getLoanDate()).c_str());
            ImGui::TableSetColumnIndex(3); ImGui::Text("%s", toString(loan.getDueDate()).c_str());

            // status/fine
            std::string statusStr = loan.isOverdue() ? "Прострочено" : "Вчасно";
            double fine = loan.calculateFine();
            if (fine > 0.0) {
                statusStr += " (штраф: " + std::to_string(static_cast<int>(fine)) + ")";
            }
            ImGui::TableSetColumnIndex(4); ImGui::Text("%s", statusStr.c_str());

            // Action: render a clickable table cell (selectable text) instead of a button, looks more natural in a table
            ImGui::TableSetColumnIndex(5);

            // Style the action like a link
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.12f, 0.48f, 0.95f, 1.0f));
            // Use Selectable without SpanAllColumns so only this cell is clickable
            std::string actionLabel = "Повернути##" + std::to_string(i);
            if (ImGui::Selectable(actionLabel.c_str(), false, 0)) {
                // perform return flow (update loan in DB and refresh)
                if (returnLoan(loan)) {
                    loadData();
                    ImGui::OpenPopup("Повернено");
                }
            }
            ImGui::PopStyleColor();
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
    // Let Loan::returnBook compute returnDate and fineAmount (it updates its internals).
    if (!loan.returnBook()) return false;

    // Let DatabaseManager handle the transactional DB update (it will also increment available_copies).
    if (!dbManager.updateLoan(loan)) return false;

    // Refresh UI data from DB to reflect the change.
    loadData();
    return true;
}

// --- ВІДОБРАЖЕННЯ ВСІХ ПОЗИЧОК ---
void LoanManager::renderLoanList() {
    // Make the Loans list look like other tables (headers, row-bg, scroll)
    auto toString = [](const std::chrono::system_clock::time_point& tp) -> std::string {
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

    // Use a 7-column table: ID, Book, Member, Loan Date, Due Date, Return Date, Status
    if (ImGui::BeginTable("LoansTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Книга");
        ImGui::TableSetupColumn("Читач");
        ImGui::TableSetupColumn("Дата позичення");
        ImGui::TableSetupColumn("Термін повернення");
        ImGui::TableSetupColumn("Дата повернення");
        ImGui::TableSetupColumn("Статус");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < loans.size(); ++i) {
            const auto& loan = loans[i];
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", loan.getId());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", getBookTitle(loan.getBookISBN()).c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", getMemberName(loan.getMemberId()).c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", toString(loan.getLoanDate()).c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", toString(loan.getDueDate()).c_str());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", toString(loan.getReturnDate()).c_str());

            ImGui::TableSetColumnIndex(6);
            std::string status = loan.getIsReturned() ? "Повернено" : (loan.isOverdue() ? "Просрочено" : "В процесі");
            double fine = loan.calculateFine();
            if (fine > 0.0 && !loan.getIsReturned()) {
                status += " (штраф: " + std::to_string(static_cast<int>(fine)) + ")";
            }
            ImGui::Text("%s", status.c_str());
        }

        ImGui::EndTable();
    }
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
