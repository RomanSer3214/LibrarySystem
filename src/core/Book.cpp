#include "Book.h"

Book::Book(const std::string& isbn, const std::string& title, 
           const std::string& author, const std::string& publisher, 
           int year, int copies)
    : isbn(isbn), title(title), author(author), publisher(publisher),
      publicationYear(year), totalCopies(copies), availableCopies(copies),
      status(Status::AVAILABLE) {}

std::string Book::getStatusString() const {
    switch(status) {
        case Status::AVAILABLE: return "Доступна";
        case Status::BORROWED: return "Позичена";
        case Status::RESERVED: return "Зарезервована";
        case Status::MAINTENANCE: return "На обслуговуванні";
        default: return "Невідомий";
    }
}

bool Book::borrowBook() {
    if (availableCopies > 0 && status == Status::AVAILABLE) {
        availableCopies--;
        if (availableCopies == 0) {
            status = Status::BORROWED;
        }
        return true;
    }
    return false;
}

bool Book::returnBook() {
    if (availableCopies < totalCopies) {
        availableCopies++;
        status = Status::AVAILABLE;
        return true;
    }
    return false;
}

bool Book::reserveBook() {
    if (status == Status::AVAILABLE) {
        status = Status::RESERVED;
        return true;
    }
    return false;
}

bool Book::isAvailable() const {
    return status == Status::AVAILABLE && availableCopies > 0;
}

void Book::setTotalCopies(int copies) {
    totalCopies = copies;
    if (availableCopies > totalCopies) {
        availableCopies = totalCopies;
    }
}
