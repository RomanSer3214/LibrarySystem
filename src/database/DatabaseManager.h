#pragma once
#include <string>
#include <vector>
#include <optional>
#include <sqlite3.h>
#include "../core/Book.h"
#include "../core/Member.h"
#include "../core/Loan.h"

class DatabaseManager {
private:
    std::string dbPath;
    sqlite3* db;

    bool executeSQL(const std::string& sql);
    bool createTables();

public:
    DatabaseManager(const std::string& path = "data/library.db");
    ~DatabaseManager();
    
    bool initialize();
   
    bool validateLogin(const std::string& username, const std::string& password, int& outUserId);

    bool addBook(const Book& book);
    bool updateBook(const Book& book);
    bool deleteBook(const std::string& isbn);
    std::vector<Book> getAllBooks();
    std::optional<Book> findBook(const std::string& isbn);
    
    bool addMember(const Member& member);
    bool updateMember(const Member& member);
    bool deleteMember(int id);
    std::vector<Member> getAllMembers();
    std::optional<Member> findMember(int id);
    
    bool addLoan(const Loan& loan);
    bool updateLoan(const Loan& loan);
    std::vector<Loan> getAllLoans();
    std::vector<Loan> getActiveLoans();
};
