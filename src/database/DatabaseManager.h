#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "../core/Book.h"
#include "../core/Member.h"
#include "../core/Loan.h"
#include "sqlite3.h"

class DatabaseManager {
private:
    std::string dbPath;
    sqlite3* db;

public:
    DatabaseManager(const std::string& path = "data/library.db");
    ~DatabaseManager();
    
    bool initialize();
    
    bool addBook(const Book& book);
    bool updateBook(const Book& book);
    bool deleteBook(const std::string& isbn);
    std::vector<Book> getAllBooks();
    Book* findBook(const std::string& isbn);
    
    bool addMember(const Member& member);
    bool updateMember(const Member& member);
    bool deleteMember(int id);
    std::vector<Member> getAllMembers();
    Member* findMember(int id);
    
    bool addLoan(const Loan& loan);
    bool updateLoan(const Loan& loan);
    std::vector<Loan> getAllLoans();
    std::vector<Loan> getActiveLoans();
    
private:
    bool executeSQL(const std::string& sql);
    bool createTables();
};
