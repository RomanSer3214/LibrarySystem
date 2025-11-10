#pragma once
#include <vector>
#include <memory>
#include "Book.h"
#include "Member.h"
#include "Loan.h"

class Library {
private:
    std::vector<Book> books;
    std::vector<Member> members;
    std::vector<Loan> loans;

public:
    void addBook(const Book& book);
    bool removeBook(const std::string& isbn);
    Book* findBook(const std::string& isbn);
    std::vector<Book> searchBooks(const std::string& query);
    
    void addMember(const Member& member);
    bool removeMember(int id);
    Member* findMember(int id);
    
    bool borrowBook(const std::string& isbn, int memberId);
    bool returnBook(const std::string& isbn, int memberId);
    std::vector<Loan> getMemberLoans(int memberId);
};
