#pragma once
#include <vector>
#include <string>
#include "../core/Book.h"
#include "../core/Member.h"
#include "../core/Loan.h"
#include "../database/DatabaseManager.h"

class LoanManager {
private:
    DatabaseManager& dbManager;
    std::vector<Loan> loans;
    std::vector<Book> books;
    std::vector<Member> members;

    char searchBuffer[256] = "";
    int selectedBookIndex = -1;
    int selectedMemberIndex = -1;
    int loanDays = 14;
    int selectedLoanIndex = -1;

    void renderLoanList();
    void renderBorrowSection();
    void renderReturnSection();
    std::string getBookTitle(const std::string& isbn) const;
    std::string getMemberName(int memberId) const;
    void addLoan();
    bool maxBooksAllowedExceeded(const Member& member) const;
    bool borrowBook(int bookIndex, int memberIndex);
    bool returnLoan(Loan& loan);

public:
    LoanManager(DatabaseManager& db);

    void render();
    void loadData();
};

