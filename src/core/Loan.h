#pragma once
#include <string>
#include <chrono>

class Loan {
private:
    int id;
    std::string bookISBN;
    int memberId;
    std::chrono::system_clock::time_point loanDate;
    std::chrono::system_clock::time_point dueDate;
    std::chrono::system_clock::time_point returnDate;
    bool isReturned;
    double fineAmount;

public:
    Loan(int id, const std::string& isbn, int memberId, int days);
    Loan(int id, const std::string& isbn, int memberId,
         std::chrono::system_clock::time_point loanDate,
         std::chrono::system_clock::time_point dueDate,
         std::chrono::system_clock::time_point returnDate,
         bool isReturned,
         double fineAmount);

    int getId() const { return id; }
    std::string getBookISBN() const { return bookISBN; }
    int getMemberId() const { return memberId; }
    auto getLoanDate() const { return loanDate; }
    auto getDueDate() const { return dueDate; }
    auto getReturnDate() const { return returnDate; }
    bool getIsReturned() const { return isReturned; }
    double getFineAmount() const { return fineAmount; }

    // --- Методи ---
    void setReturnDate(std::chrono::system_clock::time_point date) { returnDate = date; }
    void setIsReturned(bool returned) { isReturned = returned; }

    bool returnBook();

    bool isOverdue() const;
    double calculateFine() const;
};

