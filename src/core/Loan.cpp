#include "Loan.h"

Loan::Loan(const std::string& isbn, int memberId, int days)
    : bookISBN(isbn), memberId(memberId), isReturned(false), fineAmount(0.0) {
    loanDate = std::chrono::system_clock::now();
    dueDate = loanDate + std::chrono::hours(24 * days);
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

double Loan::calculateFine() const {
    if (isReturned && returnDate > dueDate) {
        auto overdueDays = std::chrono::duration_cast<std::chrono::hours>(returnDate - dueDate).count() / 24;
        return overdueDays * 5.0; // 5 грн за день прострочення
    }
    return 0.0;
}

bool Loan::isOverdue() const {
    return !isReturned && std::chrono::system_clock::now() > dueDate;
}
