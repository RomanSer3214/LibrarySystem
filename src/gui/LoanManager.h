#pragma once
#include "imgui.h"
#include <vector>
#include "../core/Loan.h"
#include "../core/Book.h"
#include "../core/Member.h"
#include "../database/DatabaseManager.h"

class LoanManager {
private:
    std::vector<Loan> loans;
    std::vector<Book> books;
    std::vector<Member> members;
    DatabaseManager& dbManager;
    
    // Стан для UI
    char bookSearchBuffer[256] = "";
    char memberSearchBuffer[256] = "";
    int selectedBookIndex = -1;
    int selectedMemberIndex = -1;
    int loanDays = 14;

public:
    LoanManager(DatabaseManager& db);
    
    void render();
    void loadData();

private:
    void renderLoanList();
    void renderBorrowSection();
    void renderReturnSection();
    
    void borrowBook();
    void returnBook(int loanIndex);
    std::string getBookTitle(const std::string& isbn);
    std::string getMemberName(int memberId);
};
