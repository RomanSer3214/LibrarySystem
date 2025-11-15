#include "Loan.h"

Loan::Loan(int id, const std::string& isbn, int memberId, int days)
    : id(id), bookISBN(isbn), memberId(memberId),
      loanDate(std::chrono::system_clock::now()),
      dueDate(loanDate + std::chrono::hours(24 * days)),
      returnDate(std::chrono::system_clock::time_point{}),
      isReturned(false), fineAmount(0.0) {}

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

// Позичення книги (позначає як повернену та розраховує штраф)
bool Loan::isOverdue() const {
    return !isReturned && std::chrono::system_clock::now() > dueDate;
}

double Loan::calculateFine() const {
    if (isReturned && returnDate > dueDate) {
        auto overdueHours = std::chrono::duration_cast<std::chrono::hours>(returnDate - dueDate).count();
        int overdueDays = static_cast<int>(overdueHours / 24);
        return overdueDays * 5.0; // 5 грн за день прострочення
    }
    return 0.0;
}

// Додатковий метод для повернення книги
bool Loan::returnBook() {
    if (!isReturned) {
        returnDate = std::chrono::system_clock::now();
        isReturned = true;
        fineAmount = calculateFine();
        return true;
    }
    return false;
}

