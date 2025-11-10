#include "Library.h"

void Library::addBook(const Book& book) {
    books.push_back(book);
}

bool Library::removeBook(const std::string& isbn) {
    for (auto it = books.begin(); it != books.end(); ++it) {
        if (it->getISBN() == isbn) {
            books.erase(it);
            return true;
        }
    }
    return false;
}

Book* Library::findBook(const std::string& isbn) {
    for (auto& book : books) {
        if (book.getISBN() == isbn) {
            return &book;
        }
    }
    return nullptr;
}

std::vector<Book> Library::searchBooks(const std::string& query) {
    std::vector<Book> results;
    for (const auto& book : books) {
        if (book.getTitle().find(query) != std::string::npos ||
            book.getAuthor().find(query) != std::string::npos ||
            book.getISBN().find(query) != std::string::npos) {
            results.push_back(book);
        }
    }
    return results;
}

void Library::addMember(const Member& member) {
    members.push_back(member);
}

bool Library::removeMember(int id) {
    for (auto it = members.begin(); it != members.end(); ++it) {
        if (it->getId() == id) {
            members.erase(it);
            return true;
        }
    }
    return false;
}

Member* Library::findMember(int id) {
    for (auto& member : members) {
        if (member.getId() == id) {
            return &member;
        }
    }
    return nullptr;
}

bool Library::borrowBook(const std::string& isbn, int memberId) {
    Book* book = findBook(isbn);
    Member* member = findMember(memberId);
    
    if (book && member && book->borrowBook()) {
        loans.emplace_back(isbn, memberId);
        return true;
    }
    return false;
}

bool Library::returnBook(const std::string& isbn, int memberId) {
    for (auto& loan : loans) {
        if (loan.getBookISBN() == isbn && loan.getMemberId() == memberId && !loan.getIsReturned()) {
            Book* book = findBook(isbn);
            if (book && loan.returnBook()) {
                book->returnBook();
                return true;
            }
        }
    }
    return false;
}

std::vector<Loan> Library::getMemberLoans(int memberId) {
    std::vector<Loan> result;
    for (const auto& loan : loans) {
        if (loan.getMemberId() == memberId) {
            result.push_back(loan);
        }
    }
    return result;
}
