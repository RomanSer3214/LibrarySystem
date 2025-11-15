#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(const std::string& path) : dbPath(path), db(nullptr) {
    initialize();
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
    return createTables();
}

bool DatabaseManager::executeSQL(const std::string& sql) {
    char* errorMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMsg) != SQLITE_OK) {
        std::cerr << "SQLite exec failed: " << (errorMsg ? errorMsg : "unknown") << std::endl;
        if (errorMsg) sqlite3_free(errorMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::createTables() {
    const char* sql = R"(
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

        CREATE TABLE IF NOT EXISTS members (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT UNIQUE,
            phone TEXT,
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

    return executeSQL(sql);
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
    const char* sql = "DELETE FROM books WHERE isbn=?";
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
    const char* sql =
        "SELECT isbn, title, author, genre, publication_year, total_copies, available_copies, status FROM books";

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

        int publicationYear = sqlite3_column_int(stmt, 4);
        int totalCopies = sqlite3_column_int(stmt, 5);
        int availableCopies = sqlite3_column_int(stmt, 6);
        int statusInt = sqlite3_column_int(stmt, 7);

        books.emplace_back(isbn, title, author, genre, publicationYear, totalCopies, availableCopies,
                           static_cast<Book::Status>(statusInt));
    }

    sqlite3_finalize(stmt);
    return books;
}

Book* DatabaseManager::findBook(const std::string& isbn) {
    const char* sql = "SELECT isbn, title, author, genre, publication_year, total_copies, available_copies, status FROM books WHERE isbn = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (findBook): " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    sqlite3_bind_text(stmt, 1, isbn.c_str(), -1, SQLITE_TRANSIENT);
    Book* result = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* t0 = sqlite3_column_text(stmt, 0);
        const unsigned char* t1 = sqlite3_column_text(stmt, 1);
        const unsigned char* t2 = sqlite3_column_text(stmt, 2);
        const unsigned char* t3 = sqlite3_column_text(stmt, 3);

        std::string s_isbn = t0 ? reinterpret_cast<const char*>(t0) : std::string();
        std::string s_title = t1 ? reinterpret_cast<const char*>(t1) : std::string();
        std::string s_author = t2 ? reinterpret_cast<const char*>(t2) : std::string();
        std::string s_genre = t3 ? reinterpret_cast<const char*>(t3) : std::string();

        int publicationYear = sqlite3_column_int(stmt, 4);
        int totalCopies = sqlite3_column_int(stmt, 5);
        int availableCopies = sqlite3_column_int(stmt, 6);
        int statusInt = sqlite3_column_int(stmt, 7);

        result = new Book(s_isbn, s_title, s_author, s_genre, publicationYear, totalCopies, availableCopies, static_cast<Book::Status>(statusInt));
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
    const char* sql = "DELETE FROM members WHERE id=?";
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

        const unsigned char* t1 = sqlite3_column_text(stmt, 1); // name
        const unsigned char* t2 = sqlite3_column_text(stmt, 2); // email
        const unsigned char* t3 = sqlite3_column_text(stmt, 3); // phone

        std::string name  = t1 ? reinterpret_cast<const char*>(t1) : "";
        std::string email = t2 ? reinterpret_cast<const char*>(t2) : "";
        std::string phone = t3 ? reinterpret_cast<const char*>(t3) : "";

        // type (0, 1, 2)
        Member::Type type =
            static_cast<Member::Type>(sqlite3_column_int(stmt, 4));

        // Створення об'єкта
        members.emplace_back(id, name, email, phone, type);
    }

    sqlite3_finalize(stmt);
    return members;
}

bool DatabaseManager::addLoan(const Loan& loan) {
    const char* sql =
        "INSERT INTO loans (id, book_isbn, member_id, loan_date, due_date, return_date, is_returned, fine_amount) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (addLoan): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // --- ПРАВИЛЬНИЙ ПОРЯДОК ---
    sqlite3_bind_int   (stmt, 1, loan.getId());
    sqlite3_bind_text  (stmt, 2, loan.getBookISBN().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int   (stmt, 3, loan.getMemberId());

    auto loan_time = std::chrono::duration_cast<std::chrono::seconds>(
                         loan.getLoanDate().time_since_epoch()).count();
    auto due_time = std::chrono::duration_cast<std::chrono::seconds>(
                        loan.getDueDate().time_since_epoch()).count();

    sqlite3_bind_int64 (stmt, 4, loan_time);
    sqlite3_bind_int64 (stmt, 5, due_time);

    if (loan.getIsReturned()) {
        auto ret_time = std::chrono::duration_cast<std::chrono::seconds>(
                            loan.getReturnDate().time_since_epoch()).count();
        sqlite3_bind_int64(stmt, 6, ret_time);
    } else {
        sqlite3_bind_null(stmt, 6);
    }

    sqlite3_bind_int   (stmt, 7, loan.getIsReturned() ? 1 : 0);
    sqlite3_bind_double(stmt, 8, loan.getFineAmount());

    // ---------------------------
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (!ok) {
        std::cerr << "SQLite step failed (addLoan): " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return ok;
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

        Loan loan(id, isbn, memberId, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24*14), {}, false, 0.0);
        // Optionally set loan id and dates if Loan supports it
        loans.push_back(loan);
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

        Loan loan(id, isbn, memberId, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24*14), {}, false, 0.0);
        loans.push_back(loan);
    }

    sqlite3_finalize(stmt);
    return loans;
}

bool DatabaseManager::updateLoan(const Loan&) {
    return false;
}

Member* DatabaseManager::findMember(int id) {
    const char* sql = "SELECT id, name, email, phone, member_type FROM members WHERE id = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed (findMember): " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }

    sqlite3_bind_int(stmt, 1, id);
    Member* result = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* t1 = sqlite3_column_text(stmt, 1);
        const unsigned char* t2 = sqlite3_column_text(stmt, 2);
        const unsigned char* t3 = sqlite3_column_text(stmt, 3);
        const unsigned char* t4 = sqlite3_column_text(stmt, 4);

        std::string name = t1 ? reinterpret_cast<const char*>(t1) : std::string();
        std::string email = t2 ? reinterpret_cast<const char*>(t2) : std::string();
        std::string phone = t3 ? reinterpret_cast<const char*>(t3) : std::string();
        std::string address = t4 ? reinterpret_cast<const char*>(t4) : std::string();

        Member::Type type = static_cast<Member::Type>(sqlite3_column_int(stmt, 5));

        result = new Member(id, name, email, phone, type);
    }
    sqlite3_finalize(stmt);
    return result;
}

