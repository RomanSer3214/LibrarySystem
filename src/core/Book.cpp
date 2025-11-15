#include "Book.h"

Book::Book(const std::string& isbn, const std::string& title, const std::string& author, const std::string& genre, int year, int copies)
    : isbn(isbn), title(title), author(author), genre(genre),
      publicationYear(year), totalCopies(copies), availableCopies(copies),
      status(Status::AVAILABLE) {}

Book::Book(const std::string& isbn, const std::string& title, const std::string& author, const std::string& genre,
           int publicationYear, int totalCopies, int availableCopies, Status status)
    : isbn(isbn), title(title), author(author), genre(genre),
      publicationYear(publicationYear), totalCopies(totalCopies),
      availableCopies(availableCopies), status(status) 
{
    if (this->totalCopies < 0) this->totalCopies = 0;
    if (this->availableCopies < 0) this->availableCopies = 0;
    if (this->availableCopies > this->totalCopies) this->availableCopies = this->totalCopies;
}

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
    if (availableCopies > 0) {
        --availableCopies;
        if (availableCopies == 0) status = Status::BORROWED;
        else status = Status::AVAILABLE; // some copies still available
        return true;
    }
    return false;
}

bool Book::returnBook() {
    if (availableCopies < totalCopies) {
        ++availableCopies;
        // if any copies available, make state AVAILABLE (could also preserve RESERVED preference)
        status = Status::AVAILABLE;
        return true;
    }
    return false;
}

bool Book::reserveBook() {
    // A simple reserve: change status to RESERVED if there are available copies.
    if (status == Status::AVAILABLE && availableCopies > 0) {
        status = Status::RESERVED;
        return true;
    }
    return false;
}

bool Book::isAvailable() const {
    return status == Status::AVAILABLE && availableCopies > 0;
}
