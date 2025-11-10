#pragma once
#include <string>
#include <chrono>

class Loan {
private:
    std::string bookISBN;
    int memberId;
    std::chrono::system_clock::time_point loanDate;
    std::chrono::system_clock::time_point dueDate;
    std::chrono::system_clock::time_point returnDate;
    bool isReturned;
    double fineAmount;

public:
    Loan(const std::string& isbn, int memberId, int days = 14);
    
    std::string getBookISBN() const { return bookISBN; }
    int getMemberId() const { return memberId; }
    auto getLoanDate() const { return loanDate; }
    auto getDueDate() const { return dueDate; }
    auto getReturnDate() const { return returnDate; }
    bool getIsReturned() const { return isReturned; }
    double getFineAmount() const { return fineAmount; }
    
    bool returnBook();
    double calculateFine() const;
    bool isOverdue() const;
};
