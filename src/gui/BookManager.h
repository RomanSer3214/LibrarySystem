#pragma once
#include <vector>
#include <string>
#include "../core/Book.h"
#include "../database/DatabaseManager.h"

class BookManager {
private:
    DatabaseManager& dbManager;
    std::vector<Book> books;
    
    char searchBuffer[256] = "";
    char isbnBuffer[64] = "";
    char titleBuffer[256] = "";
    char authorBuffer[256] = "";
    char genreBuffer[256] = "";
    int publicationYear = 2025;
    int totalCopies = 1;
    
    bool showAddBookPopup = false;
    bool showEditBookPopup = false;
    int selectedBookIndex = -1;

    void renderBookList();
    void renderAddBookPopup();
    void renderEditBookPopup();
    void renderSearchBar();
    
    void addBook();
    void editBook();
    void deleteBook(int index);
    void searchBooks(const std::string& query);

public:
    BookManager(DatabaseManager& db);
    
    void render();
    void loadBooks();
    void saveBooks();
};
