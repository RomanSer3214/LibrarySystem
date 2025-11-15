#include "Loan.h"
#include <chrono>
#include <cassert>

// Constructors
Loan::Loan(int id, const std::string& isbn, int memberId, int days)
    : id(id), bookISBN(isbn), memberId(memberId),
      loanDate(std::chrono::system_clock::now()),
      dueDate(std::chrono::system_clock::now() + std::chrono::hours(24 * days)),
      returnDate(), isReturned(false), fineAmount(0.0) {}

Loan::Loan(int id, const std::string& isbn, int memberId,
           std::chrono::system_clock::time_point loanDate,
           std::chrono::system_clock::time_point dueDate,
           std::chrono::system_clock::time_point returnDate,
           bool isReturned,
           double fineAmount)
    : id(id), bookISBN(isbn), memberId(memberId),
      loanDate(loanDate), dueDate(dueDate),
      returnDate(returnDate), isReturned(isReturned),
      fineAmount(fineAmount) {}

// Check if loan is overdue relative to now (or relative to returnDate if returned)
bool Loan::isOverdue() const {
    if (isReturned) {
        // If already returned, overdue only if returnDate > dueDate
        return returnDate > dueDate;
    } else {
        return std::chrono::system_clock::now() > dueDate;
    }
}

double Loan::calculateFine() const {
    // Calculate fine as 5.0 per overdue day (same as prior logic)
    std::chrono::system_clock::time_point comparePoint;
    if (isReturned) {
        comparePoint = returnDate;
    } else {
        comparePoint = std::chrono::system_clock::now();
    }

    if (comparePoint <= dueDate) return 0.0;

    auto overdueHours = std::chrono::duration_cast<std::chrono::hours>(comparePoint - dueDate).count();
    int overdueDays = static_cast<int>((overdueHours + 23) / 24); // round up partial days
    double fine = overdueDays * 5.0; // 5 units per day
    return fine;
}

bool Loan::returnBook() {
    if (!isReturned) {
        returnDate = std::chrono::system_clock::now();
        isReturned = true;
        fineAmount = calculateFine();
        return true;
    }
    return false;
}
