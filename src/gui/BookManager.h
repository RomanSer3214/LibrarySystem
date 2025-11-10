#pragma once
#include <vector>
#include <string>
#include "../../libs/imgui/imgui.h"
#include "../core/Book.h"
#include "../database/DatabaseManager.h"

class BookManager {
private:
    std::vector<Book> books;
    DatabaseManager& dbManager;
    
    // Стан для UI
    char searchBuffer[256] = "";
    char isbnBuffer[64] = "";
    char titleBuffer[256] = "";
    char authorBuffer[256] = "";
    char publisherBuffer[256] = "";
    int publicationYear = 2024;
    int totalCopies = 1;
    
    bool showAddBookPopup = false;
    bool showEditBookPopup = false;
    int selectedBookIndex = -1;

public:
    BookManager(DatabaseManager& db);
    
    void render();
    void loadBooks();
    void saveBooks();

private:
    void renderBookList();
    void renderAddBookPopup();
    void renderEditBookPopup();
    void renderSearchBar();
    
    void addBook();
    void editBook();
    void deleteBook(int index);
    void searchBooks(const std::string& query);
};
