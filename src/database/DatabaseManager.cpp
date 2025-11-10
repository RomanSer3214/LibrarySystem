#include "DatabaseManager.h"
#include <iostream>
#include <sqlite3.h>

DatabaseManager::DatabaseManager(const std::string& path) : dbPath(path), db(nullptr) {
    initialize();
}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(static_cast<sqlite3*>(db));
    }
}

bool DatabaseManager::initialize() {
    sqlite3* sqlite_db;
    if (sqlite3_open(dbPath.c_str(), &sqlite_db) != SQLITE_OK) {
        std::cerr << "Не вдалося відкрити базу даних: " << sqlite3_errmsg(sqlite_db) << std::endl;
        return false;
    }
    
    db = sqlite_db;  // Зберігаємо як void*
    return createTables();
}

bool DatabaseManager::createTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS books (
            isbn TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            author TEXT NOT NULL,
            publisher TEXT,
            publication_year INTEGER,
            total_copies INTEGER,
            available_copies INTEGER,
            genre TEXT,
            status INTEGER
        );
        
        CREATE TABLE IF NOT EXISTS members (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT UNIQUE,
            phone TEXT,
            address TEXT,
            member_type INTEGER,
            max_books_allowed INTEGER
        );
        
        CREATE TABLE IF NOT EXISTS loans (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            book_isbn TEXT NOT NULL,
            member_id INTEGER NOT NULL,
            loan_date INTEGER NOT NULL,
            due_date INTEGER NOT NULL,
            return_date INTEGER,
            is_returned INTEGER DEFAULT 0,
            fine_amount REAL DEFAULT 0.0,
            FOREIGN KEY (book_isbn) REFERENCES books (isbn),
            FOREIGN KEY (member_id) REFERENCES members (id)
        );
    )";
    
    char* errorMsg = nullptr;
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    
    if (sqlite3_exec(sqlite_db, sql, nullptr, nullptr, &errorMsg) != SQLITE_OK) {
        std::cerr << "Помилка створення таблиць: " << errorMsg << std::endl;
        sqlite3_free(errorMsg);
        return false;
    }
    
    return true;
}

bool DatabaseManager::addBook(const Book& book) {
    const char* sql = "INSERT INTO books (isbn, title, author, publisher, publication_year, total_copies, available_copies, genre, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, book.getISBN().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, book.getTitle().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, book.getAuthor().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, book.getPublisher().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, book.getPublicationYear());
    sqlite3_bind_int(stmt, 6, book.getTotalCopies());
    sqlite3_bind_int(stmt, 7, book.getAvailableCopies());
    sqlite3_bind_text(stmt, 8, "", -1, SQLITE_STATIC); // genre
    sqlite3_bind_int(stmt, 9, static_cast<int>(book.getStatus()));
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

std::vector<Book> DatabaseManager::getAllBooks() {
    std::vector<Book> books;
    const char* sql = "SELECT isbn, title, author, publisher, publication_year, total_copies, available_copies, status FROM books";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return books;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string isbn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string publisher = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int year = sqlite3_column_int(stmt, 4);
        int total = sqlite3_column_int(stmt, 5);
        int available = sqlite3_column_int(stmt, 6);
        
        books.emplace_back(isbn, title, author, publisher, year, total);
    }
    
    sqlite3_finalize(stmt);
    return books;
}

bool DatabaseManager::addMember(const Member& member) {
    const char* sql = "INSERT INTO members (name, email, phone, address, member_type, max_books_allowed) VALUES (?, ?, ?, ?, ?, ?)";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, member.getName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, member.getEmail().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, member.getPhone().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, "", -1, SQLITE_STATIC); // address
    sqlite3_bind_int(stmt, 5, static_cast<int>(member.getType()));
    sqlite3_bind_int(stmt, 6, member.getMaxBooksAllowed());
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

std::vector<Member> DatabaseManager::getAllMembers() {
    std::vector<Member> members;
    const char* sql = "SELECT id, name, email, phone, member_type FROM members";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return members;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        Member::Type type = static_cast<Member::Type>(sqlite3_column_int(stmt, 4));
        
        members.emplace_back(id, name, email, type);
    }
    
    sqlite3_finalize(stmt);
    return members;
}

bool DatabaseManager::updateBook(const Book& book) {
    const char* sql = "UPDATE books SET title=?, author=?, publisher=?, publication_year=?, total_copies=?, available_copies=?, status=? WHERE isbn=?";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, book.getTitle().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, book.getAuthor().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, book.getPublisher().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, book.getPublicationYear());
    sqlite3_bind_int(stmt, 5, book.getTotalCopies());
    sqlite3_bind_int(stmt, 6, book.getAvailableCopies());
    sqlite3_bind_int(stmt, 7, static_cast<int>(book.getStatus()));
    sqlite3_bind_text(stmt, 8, book.getISBN().c_str(), -1, SQLITE_STATIC);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

bool DatabaseManager::deleteBook(const std::string& isbn) {
    const char* sql = "DELETE FROM books WHERE isbn=?";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, isbn.c_str(), -1, SQLITE_STATIC);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

bool DatabaseManager::deleteMember(int id) {
    const char* sql = "DELETE FROM members WHERE id=?";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

bool DatabaseManager::addLoan(const Loan& loan) {
    const char* sql = "INSERT INTO loans (book_isbn, member_id, loan_date, due_date) VALUES (?, ?, ?, ?)";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, loan.getBookISBN().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, loan.getMemberId());
    
    // Конвертуємо time_point в timestamp
    auto loan_time = std::chrono::duration_cast<std::chrono::seconds>(loan.getLoanDate().time_since_epoch()).count();
    auto due_time = std::chrono::duration_cast<std::chrono::seconds>(loan.getDueDate().time_since_epoch()).count();
    
    sqlite3_bind_int64(stmt, 3, loan_time);
    sqlite3_bind_int64(stmt, 4, due_time);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

std::vector<Loan> DatabaseManager::getAllLoans() {
    std::vector<Loan> loans;
    const char* sql = "SELECT book_isbn, member_id, loan_date, due_date, return_date, is_returned, fine_amount FROM loans";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return loans;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string isbn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int memberId = sqlite3_column_int(stmt, 1);
        
        // Створюємо базовий Loan
        Loan loan(isbn, memberId);
        
        loans.push_back(loan);
    }
    
    sqlite3_finalize(stmt);
    return loans;
}

std::vector<Loan> DatabaseManager::getActiveLoans() {
    std::vector<Loan> loans;
    const char* sql = "SELECT book_isbn, member_id, loan_date, due_date FROM loans WHERE is_returned = 0";
    
    sqlite3* sqlite_db = static_cast<sqlite3*>(db);
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return loans;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string isbn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int memberId = sqlite3_column_int(stmt, 1);
        
        Loan loan(isbn, memberId);
        loans.push_back(loan);
    }
    
    sqlite3_finalize(stmt);
    return loans;
}
