#include "DatabaseManager.h"
#include <iostream>
#include <chrono>
#include <optional>
#include "../core/hash.h"

static inline std::int64_t timepoint_to_seconds(const std::chrono::system_clock::time_point& tp) {
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}
static inline std::chrono::system_clock::time_point seconds_to_timepoint(std::int64_t s) {
    return std::chrono::system_clock::time_point(std::chrono::seconds(s));
}

DatabaseManager::DatabaseManager(const std::string& path) : dbPath(path), db(nullptr) {
    initialize();
}

bool DatabaseManager::validateLogin(const std::string& username, const std::string& password, int& outUserId) {
    sqlite3_stmt* stmt = nullptr;
    std::string query = "SELECT id, password_hash FROM users WHERE username = ? LIMIT 1";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (validateLogin): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        std::cerr << "User not found or step failed: rc=" << rc << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    int userId = sqlite3_column_int(stmt, 0);
    const unsigned char* storedHashC = sqlite3_column_text(stmt, 1);
    std::string storedHash = storedHashC ? reinterpret_cast<const char*>(storedHashC) : "";

    sqlite3_finalize(stmt);

    std::string inputHash = sha256(password);

    storedHash.erase(std::remove_if(storedHash.begin(), storedHash.end(), ::isspace), storedHash.end());

    if (inputHash == storedHash) {
        outUserId = userId;
        std::cout << "Login successful! User ID: " << userId << "\n";
        return true;
    }

    std::cout << "Login failed inside validateLogin\n";
    return false;
}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool DatabaseManager::initialize() {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Не вдалося відкрити базу даних: "
                  << (db ? sqlite3_errmsg(db) : "unknown") << std::endl;
        return false;
    }

    // Ensure foreign keys enforced
    if (!executeSQL("PRAGMA foreign_keys = ON;")) {
        std::cerr << "Could not enable PRAGMA foreign_keys" << std::endl;
    }

    return createTables();
}

bool DatabaseManager::executeSQL(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "SQLite exec failed: " << (err ? err : "unknown") << std::endl;
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

bool DatabaseManager::createTables() {
    const char* books_sql = R"(
        CREATE TABLE IF NOT EXISTS books (
            isbn TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            author TEXT NOT NULL,
            genre TEXT,
            publication_year INTEGER,
            total_copies INTEGER,
            available_copies INTEGER,
            status INTEGER
        );
    )";

    const char* members_sql = R"(
        CREATE TABLE IF NOT EXISTS members (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT UNIQUE,
            phone TEXT,
            member_type INTEGER,
            max_books_allowed INTEGER
        );
    )";

    const char* loans_sql = R"(
        CREATE TABLE IF NOT EXISTS loans (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            book_isbn TEXT NOT NULL,
            member_id INTEGER NOT NULL,
            loan_date INTEGER NOT NULL,
            due_date INTEGER NOT NULL,
            return_date INTEGER,
            is_returned INTEGER DEFAULT 0,
            fine_amount REAL DEFAULT 0.0,
            FOREIGN KEY (book_isbn) REFERENCES books (isbn) ON DELETE CASCADE,
            FOREIGN KEY (member_id) REFERENCES members (id) ON DELETE CASCADE
        );
    )";

    const char* users_sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL
        );
    )";

    if (!(executeSQL(books_sql) && executeSQL(members_sql) && executeSQL(loans_sql) && executeSQL(users_sql))) {
        return false;
    }

    const char* insertAdmin = R"(
        INSERT INTO users (username, password_hash)
        VALUES ('admin', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918')
        ON CONFLICT(username) DO UPDATE SET password_hash=excluded.password_hash;
    )";

    return executeSQL(insertAdmin);
}

bool DatabaseManager::addBook(const Book& book) {
    const char* sql =
        "INSERT INTO books (isbn, title, author, genre, publication_year, total_copies, available_copies, status) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (addBook): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, book.getISBN().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, book.getAuthor().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, book.getGenre().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, book.getPublicationYear());
    sqlite3_bind_int(stmt, 6, book.getTotalCopies());
    sqlite3_bind_int(stmt, 7, book.getAvailableCopies());
    sqlite3_bind_int(stmt, 8, static_cast<int>(book.getStatus()));

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) std::cerr << "SQLite step failed (addBook): " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    return ok;
}

bool DatabaseManager::updateBook(const Book& book) {
    const char* sql =
        "UPDATE books SET title=?, author=?, genre=?, publication_year=?, total_copies=?, available_copies=?, status=? WHERE isbn=?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (updateBook): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, book.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.getAuthor().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, book.getGenre().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, book.getPublicationYear());
    sqlite3_bind_int(stmt, 5, book.getTotalCopies());
    sqlite3_bind_int(stmt, 6, book.getAvailableCopies());
    sqlite3_bind_int(stmt, 7, static_cast<int>(book.getStatus()));
    sqlite3_bind_text(stmt, 8, book.getISBN().c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) std::cerr << "SQLite step failed (updateBook): " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    return ok;
}

bool DatabaseManager::deleteBook(const std::string& isbn) {
    const char* sql = "DELETE FROM books WHERE isbn = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (deleteBook): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, isbn.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) std::cerr << "SQLite step failed (deleteBook): " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<Book> DatabaseManager::getAllBooks() {
    std::vector<Book> books;
    const char* sql = "SELECT isbn, title, author, genre, publication_year, total_copies, available_copies, status FROM books";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (getAllBooks): " << sqlite3_errmsg(db) << std::endl;
        return books;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* t0 = sqlite3_column_text(stmt, 0);
        const unsigned char* t1 = sqlite3_column_text(stmt, 1);
        const unsigned char* t2 = sqlite3_column_text(stmt, 2);
        const unsigned char* t3 = sqlite3_column_text(stmt, 3);
        std::string isbn = t0 ? reinterpret_cast<const char*>(t0) : std::string();
        std::string title = t1 ? reinterpret_cast<const char*>(t1) : std::string();
        std::string author = t2 ? reinterpret_cast<const char*>(t2) : std::string();
        std::string genre = t3 ? reinterpret_cast<const char*>(t3) : std::string();
        int pub = sqlite3_column_int(stmt, 4);
        int total = sqlite3_column_int(stmt, 5);
        int avail = sqlite3_column_int(stmt, 6);
        int statusInt = sqlite3_column_int(stmt, 7);
        books.emplace_back(isbn, title, author, genre, pub, total, avail, static_cast<Book::Status>(statusInt));
    }
    sqlite3_finalize(stmt);
    return books;
}

std::optional<Book> DatabaseManager::findBook(const std::string& isbn) {
    const char* sql = "SELECT isbn, title, author, genre, publication_year, total_copies, available_copies, status FROM books WHERE isbn = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (findBook): " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }
    sqlite3_bind_text(stmt, 1, isbn.c_str(), -1, SQLITE_TRANSIENT);
    std::optional<Book> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* t0 = sqlite3_column_text(stmt, 0);
        const unsigned char* t1 = sqlite3_column_text(stmt, 1);
        const unsigned char* t2 = sqlite3_column_text(stmt, 2);
        const unsigned char* t3 = sqlite3_column_text(stmt, 3);
        std::string s_isbn = t0 ? reinterpret_cast<const char*>(t0) : std::string();
        std::string title = t1 ? reinterpret_cast<const char*>(t1) : std::string();
        std::string author = t2 ? reinterpret_cast<const char*>(t2) : std::string();
        std::string genre = t3 ? reinterpret_cast<const char*>(t3) : std::string();
        int pub = sqlite3_column_int(stmt, 4);
        int total = sqlite3_column_int(stmt, 5);
        int avail = sqlite3_column_int(stmt, 6);
        int statusInt = sqlite3_column_int(stmt, 7);
        result = Book(s_isbn, title, author, genre, pub, total, avail, static_cast<Book::Status>(statusInt));
    }
    sqlite3_finalize(stmt);
    return result;
}


bool DatabaseManager::addMember(const Member& member) {
    const char* sql = "INSERT INTO members (name, email, phone, member_type, max_books_allowed) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (addMember): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, member.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, member.getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, member.getPhone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, static_cast<int>(member.getType()));
    sqlite3_bind_int(stmt, 5, member.getMaxBooksAllowed());
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) std::cerr << "SQLite step failed (addMember): " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    return ok;
}

bool DatabaseManager::updateMember(const Member& member) {
    const char* sql = "UPDATE members SET name=?, email=?, phone=?, member_type=?, max_books_allowed=? WHERE id=?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (updateMember): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, member.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, member.getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, member.getPhone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, static_cast<int>(member.getType()));
    sqlite3_bind_int(stmt, 5, member.getMaxBooksAllowed());
    sqlite3_bind_int(stmt, 6, member.getId());
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) std::cerr << "SQLite step failed (updateMember): " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    return ok;
}

bool DatabaseManager::deleteMember(int id) {
    const char* sql = "DELETE FROM members WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (deleteMember): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) std::cerr << "SQLite step failed (deleteMember): " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<Member> DatabaseManager::getAllMembers() {
    std::vector<Member> members;
    const char* sql = "SELECT id, name, email, phone, member_type, max_books_allowed FROM members";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (getAllMembers): " << sqlite3_errmsg(db) << std::endl;
        return members;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* t1 = sqlite3_column_text(stmt, 1);
        const unsigned char* t2 = sqlite3_column_text(stmt, 2);
        const unsigned char* t3 = sqlite3_column_text(stmt, 3);
        std::string name = t1 ? reinterpret_cast<const char*>(t1) : std::string();
        std::string email = t2 ? reinterpret_cast<const char*>(t2) : std::string();
        std::string phone = t3 ? reinterpret_cast<const char*>(t3) : std::string();
        Member::Type type = static_cast<Member::Type>(sqlite3_column_int(stmt, 4));
        int maxAllowed = sqlite3_column_int(stmt, 5);
        members.emplace_back(id, name, email, phone, type, maxAllowed);
    }
    sqlite3_finalize(stmt);
    return members;
}

std::optional<Member> DatabaseManager::findMember(int id) {
    const char* sql = "SELECT id, name, email, phone, member_type, max_books_allowed FROM members WHERE id = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (findMember): " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }
    sqlite3_bind_int(stmt, 1, id);
    std::optional<Member> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int mid = sqlite3_column_int(stmt, 0);
        const unsigned char* t1 = sqlite3_column_text(stmt, 1);
        const unsigned char* t2 = sqlite3_column_text(stmt, 2);
        const unsigned char* t3 = sqlite3_column_text(stmt, 3);
        std::string name = t1 ? reinterpret_cast<const char*>(t1) : std::string();
        std::string email = t2 ? reinterpret_cast<const char*>(t2) : std::string();
        std::string phone = t3 ? reinterpret_cast<const char*>(t3) : std::string();
        Member::Type type = static_cast<Member::Type>(sqlite3_column_int(stmt, 4));
        int maxAllowed = sqlite3_column_int(stmt, 5);
        result = Member(mid, name, email, phone, type, maxAllowed);
    }
    sqlite3_finalize(stmt);
    return result;
}


bool DatabaseManager::addLoan(const Loan& loan) {
    // Use transaction: insert loan and decrement book.available_copies
    if (!executeSQL("BEGIN TRANSACTION;")) {
        std::cerr << "Failed to begin transaction for addLoan" << std::endl;
        return false;
    }

    // Check book exists and has available copies
    auto bookOpt = findBook(loan.getBookISBN());
    if (!bookOpt.has_value()) {
        std::cerr << "Book not found when adding loan: " << loan.getBookISBN() << std::endl;
        executeSQL("ROLLBACK;");
        return false;
    }
    Book book = bookOpt.value();
    if (book.getAvailableCopies() <= 0) {
        std::cerr << "No available copies for book when adding loan: " << loan.getBookISBN() << std::endl;
        executeSQL("ROLLBACK;");
        return false;
    }

    const char* sql =
        "INSERT INTO loans (book_isbn, member_id, loan_date, due_date, return_date, is_returned, fine_amount) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (addLoan): " << sqlite3_errmsg(db) << std::endl;
        executeSQL("ROLLBACK;");
        return false;
    }

    sqlite3_bind_text(stmt, 1, loan.getBookISBN().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, loan.getMemberId());
    sqlite3_bind_int64(stmt, 3, timepoint_to_seconds(loan.getLoanDate()));
    sqlite3_bind_int64(stmt, 4, timepoint_to_seconds(loan.getDueDate()));
    if (loan.getIsReturned()) sqlite3_bind_int64(stmt, 5, timepoint_to_seconds(loan.getReturnDate()));
    else sqlite3_bind_null(stmt, 5);
    sqlite3_bind_int(stmt, 6, loan.getIsReturned() ? 1 : 0);
    sqlite3_bind_double(stmt, 7, loan.getFineAmount());

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) {
        std::cerr << "SQLite step failed (addLoan): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        executeSQL("ROLLBACK;");
        return false;
    }
    sqlite3_finalize(stmt);

    // decrement available_copies and update book row
    book.setAvailableCopies(book.getAvailableCopies() - 1);
    if (book.getAvailableCopies() < 0) book.setAvailableCopies(0);
    if (book.getAvailableCopies() == 0) book.setStatus(Book::Status::BORROWED);
    else book.setStatus(Book::Status::AVAILABLE);

    if (!updateBook(book)) {
        std::cerr << "Failed to update book after adding loan" << std::endl;
        executeSQL("ROLLBACK;");
        return false;
    }

    if (!executeSQL("COMMIT;")) {
        std::cerr << "Failed to commit addLoan transaction" << std::endl;
        executeSQL("ROLLBACK;");
        return false;
    }

    return true;
}

std::vector<Loan> DatabaseManager::getAllLoans() {
    std::vector<Loan> loans;
    const char* sql = "SELECT id, book_isbn, member_id, loan_date, due_date, return_date, is_returned, fine_amount FROM loans";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (getAllLoans): " << sqlite3_errmsg(db) << std::endl;
        return loans;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* t1 = sqlite3_column_text(stmt, 1);
        std::string isbn = t1 ? reinterpret_cast<const char*>(t1) : std::string();
        int memberId = sqlite3_column_int(stmt, 2);
        std::int64_t loan_time = sqlite3_column_int64(stmt, 3);
        std::int64_t due_time = sqlite3_column_int64(stmt, 4);
        std::chrono::system_clock::time_point loanDate = seconds_to_timepoint(loan_time);
        std::chrono::system_clock::time_point dueDate = seconds_to_timepoint(due_time);

        std::chrono::system_clock::time_point returnDate{};
        bool isReturned = sqlite3_column_int(stmt, 6) != 0;
        if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
            std::int64_t ret_time = sqlite3_column_int64(stmt, 5);
            returnDate = seconds_to_timepoint(ret_time);
        }

        double fine = sqlite3_column_double(stmt, 7);
        loans.emplace_back(id, isbn, memberId, loanDate, dueDate, returnDate, isReturned, fine);
    }
    sqlite3_finalize(stmt);
    return loans;
}

std::vector<Loan> DatabaseManager::getActiveLoans() {
    std::vector<Loan> loans;
    const char* sql = "SELECT id, book_isbn, member_id, loan_date, due_date FROM loans WHERE is_returned = 0";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (getActiveLoans): " << sqlite3_errmsg(db) << std::endl;
        return loans;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* t1 = sqlite3_column_text(stmt, 1);
        std::string isbn = t1 ? reinterpret_cast<const char*>(t1) : std::string();
        int memberId = sqlite3_column_int(stmt, 2);
        std::int64_t loan_time = sqlite3_column_int64(stmt, 3);
        std::int64_t due_time = sqlite3_column_int64(stmt, 4);
        std::chrono::system_clock::time_point loanDate = seconds_to_timepoint(loan_time);
        std::chrono::system_clock::time_point dueDate = seconds_to_timepoint(due_time);
        loans.emplace_back(id, isbn, memberId, loanDate, dueDate, std::chrono::system_clock::time_point{}, false, 0.0);
    }
    sqlite3_finalize(stmt);
    return loans;
}

bool DatabaseManager::updateLoan(const Loan& loan) {
    // Use transaction: update loan, and if transitioning to returned increment book.available_copies
    if (!executeSQL("BEGIN TRANSACTION;")) {
        std::cerr << "Failed to begin transaction (updateLoan)" << std::endl;
        return false;
    }

    // Find current loan state
    const char* sel_sql = "SELECT is_returned, book_isbn FROM loans WHERE id = ? LIMIT 1";
    sqlite3_stmt* sel_stmt = nullptr;
    if (sqlite3_prepare_v2(db, sel_sql, -1, &sel_stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (updateLoan select): " << sqlite3_errmsg(db) << std::endl;
        executeSQL("ROLLBACK;");
        return false;
    }
    sqlite3_bind_int(sel_stmt, 1, loan.getId());
    if (sqlite3_step(sel_stmt) != SQLITE_ROW) {
        std::cerr << "Loan not found for updateLoan id=" << loan.getId() << std::endl;
        sqlite3_finalize(sel_stmt);
        executeSQL("ROLLBACK;");
        return false;
    }
    int old_is_returned = sqlite3_column_int(sel_stmt, 0);
    const unsigned char* t_isbn = sqlite3_column_text(sel_stmt, 1);
    std::string isbn = t_isbn ? reinterpret_cast<const char*>(t_isbn) : std::string();
    sqlite3_finalize(sel_stmt);

    // Update loan row
    const char* upd_sql = "UPDATE loans SET return_date = ?, is_returned = ?, fine_amount = ? WHERE id = ?";
    sqlite3_stmt* upd_stmt = nullptr;
    if (sqlite3_prepare_v2(db, upd_sql, -1, &upd_stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (updateLoan update): " << sqlite3_errmsg(db) << std::endl;
        executeSQL("ROLLBACK;");
        return false;
    }

    if (loan.getIsReturned()) {
        sqlite3_bind_int64(upd_stmt, 1, timepoint_to_seconds(loan.getReturnDate()));
    } else {
        sqlite3_bind_null(upd_stmt, 1);
    }
    sqlite3_bind_int(upd_stmt, 2, loan.getIsReturned() ? 1 : 0);
    sqlite3_bind_double(upd_stmt, 3, loan.getFineAmount());
    sqlite3_bind_int(upd_stmt, 4, loan.getId());

    if (sqlite3_step(upd_stmt) != SQLITE_DONE) {
        std::cerr << "SQLite step failed (updateLoan update): " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(upd_stmt);
        executeSQL("ROLLBACK;");
        return false;
    }
    sqlite3_finalize(upd_stmt);

    // If transitioning from not returned -> returned, increment book.available_copies
    if (old_is_returned == 0 && loan.getIsReturned()) {
        auto bookOpt = findBook(isbn);
        if (!bookOpt.has_value()) {
            std::cerr << "Book not found while updating loan: " << isbn << std::endl;
            executeSQL("ROLLBACK;");
            return false;
        }
        Book book = bookOpt.value();
        book.setAvailableCopies(book.getAvailableCopies() + 1);
        if (book.getAvailableCopies() > 0) book.setStatus(Book::Status::AVAILABLE);
        if (!updateBook(book)) {
            std::cerr << "Failed to update book while updating loan" << std::endl;
            executeSQL("ROLLBACK;");
            return false;
        }
    }

    if (!executeSQL("COMMIT;")) {
        std::cerr << "Failed to commit updateLoan transaction" << std::endl;
        executeSQL("ROLLBACK;");
        return false;
    }

    return true;
}
