#include <cstring>
#include "BookManager.h"
#include "imgui.h"

BookManager::BookManager(DatabaseManager& db) : dbManager(db) {}

void BookManager::render() {
    loadBooks();
    renderSearchBar();
    
    ImGui::Separator();
    
    if (ImGui::Button("Додати книгу")) {
        showAddBookPopup = true;
        memset(isbnBuffer, 0, sizeof(isbnBuffer));
        memset(titleBuffer, 0, sizeof(titleBuffer));
        memset(authorBuffer, 0, sizeof(authorBuffer));
        memset(genreBuffer, 0, sizeof(genreBuffer));
        publicationYear = 2025;
        totalCopies = 1;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Редагувати") && selectedBookIndex >= 0) {
        showEditBookPopup = true;
        const auto& book = books[selectedBookIndex];
        
        strncpy(isbnBuffer, book.getISBN().c_str(), sizeof(isbnBuffer) - 1);
        strncpy(titleBuffer, book.getTitle().c_str(), sizeof(titleBuffer) - 1);
        strncpy(authorBuffer, book.getAuthor().c_str(), sizeof(authorBuffer) - 1);
        strncpy(genreBuffer, book.getGenre().c_str(), sizeof(genreBuffer) - 1);
        
        isbnBuffer[sizeof(isbnBuffer) - 1] = '\0';
        titleBuffer[sizeof(titleBuffer) - 1] = '\0';
        authorBuffer[sizeof(authorBuffer) - 1] = '\0';
        genreBuffer[sizeof(genreBuffer) - 1] = '\0';
        
        publicationYear = book.getPublicationYear();
        totalCopies = book.getTotalCopies();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Видалити") && selectedBookIndex >= 0) {
        deleteBook(selectedBookIndex);
    }
    
    ImGui::Spacing();
    
    renderBookList();
    
    if (showAddBookPopup) {
        renderAddBookPopup();
    }
    
    if (showEditBookPopup) {
        renderEditBookPopup();
    }
}

void BookManager::renderSearchBar() {
    ImGui::Text("Пошук:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(400);   
    ImGui::InputText("##search", searchBuffer, sizeof(searchBuffer));
    if (strlen(searchBuffer) > 0) {
        searchBooks(searchBuffer);
    }
}

void BookManager::renderBookList() {
    if (ImGui::BeginTable("BooksTable", 6, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("ISBN");
        ImGui::TableSetupColumn("Назва");
        ImGui::TableSetupColumn("Автор");
        ImGui::TableSetupColumn("Рік");
        ImGui::TableSetupColumn("Доступно");
        ImGui::TableSetupColumn("Статус");
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(books.size()); i++) {
            const auto& book = books[i];

            ImGui::TableNextRow();

            // Вся рядок клікабельна через Selectable
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(book.getISBN().c_str(), selectedBookIndex == i, ImGuiSelectableFlags_SpanAllColumns)) {
                selectedBookIndex = i;
            }

            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", book.getTitle().c_str());
            ImGui::TableSetColumnIndex(2); ImGui::Text("%s", book.getAuthor().c_str());
            ImGui::TableSetColumnIndex(3); ImGui::Text("%d", book.getPublicationYear());
            ImGui::TableSetColumnIndex(4); ImGui::Text("%d/%d", book.getAvailableCopies(), book.getTotalCopies());
            ImGui::TableSetColumnIndex(5); ImGui::Text("%s", book.getStatusString().c_str());
        }

        ImGui::EndTable();
    }
}

void BookManager::renderAddBookPopup() {
    static std::string addBookError;
    static bool showAddBookError = false;

    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
    ImGui::OpenPopup("Додати книгу");
    if (ImGui::BeginPopupModal("Додати книгу", &showAddBookPopup, ImGuiWindowFlags_NoResize)) {
        ImGui::InputText("ISBN", isbnBuffer, sizeof(isbnBuffer));
        ImGui::InputText("Назва", titleBuffer, sizeof(titleBuffer));
        ImGui::InputText("Автор", authorBuffer, sizeof(authorBuffer));
        ImGui::InputText("Жанр", genreBuffer, sizeof(genreBuffer));
        ImGui::InputInt("Рік публікації", &publicationYear);
        ImGui::InputInt("Кількість копій", &totalCopies);
        
        if (ImGui::Button("Зберегти")) {
            // validation
            std::string isbn = isbnBuffer;
            std::string title = titleBuffer;
            std::string author = authorBuffer;

            if (isbn.empty() || title.empty() || author.empty()) {
                addBookError = "ISBN, Назва та Автор обов'язкові.";
                showAddBookError = true;
            } else if (totalCopies <= 0) {
                addBookError = "Кількість копій повинна бути мінімум 1.";
                showAddBookError = true;
            } else {
                addBook();
                showAddBookPopup = false;
                showAddBookError = false;
            }

            if (showAddBookError) ImGui::OpenPopup("Помилка додавання книги");
        }
        ImGui::SameLine();
        if (ImGui::Button("Скасувати")) {
            showAddBookPopup = false;
        }

        if (ImGui::BeginPopupModal("Помилка додавання книги", &showAddBookError, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", addBookError.c_str());
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                showAddBookError = false;
            }
            ImGui::EndPopup();
        }
        
        ImGui::EndPopup();
    }
}

void BookManager::renderEditBookPopup() {
    static std::string editBookError;
    static bool showEditBookError = false;

    if (selectedBookIndex < 0) return; 
    Book& book = books[selectedBookIndex];

    if (showEditBookPopup && !ImGui::IsPopupOpen("Редагувати книгу")) {
        strncpy(isbnBuffer, book.getISBN().c_str(), sizeof(isbnBuffer));
        strncpy(titleBuffer, book.getTitle().c_str(), sizeof(titleBuffer));
        strncpy(authorBuffer, book.getAuthor().c_str(), sizeof(authorBuffer));
        strncpy(genreBuffer, book.getGenre().c_str(), sizeof(genreBuffer));
        publicationYear = book.getPublicationYear();
        totalCopies = book.getTotalCopies();

        ImGui::OpenPopup("Редагувати книгу");
    }

    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Редагувати книгу", &showEditBookPopup,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        // make ISBN read-only to avoid primary-key problems
        ImGui::InputText("ISBN", isbnBuffer, sizeof(isbnBuffer), ImGuiInputTextFlags_ReadOnly);
        ImGui::InputText("Назва", titleBuffer, sizeof(titleBuffer));
        ImGui::InputText("Автор", authorBuffer, sizeof(authorBuffer));
        ImGui::InputText("Видавництво", genreBuffer, sizeof(genreBuffer));
        ImGui::InputInt("Рік публікації", &publicationYear);
        ImGui::InputInt("Кількість копій", &totalCopies);

        ImGui::Separator();

        if (ImGui::Button("Зберегти")) {
            // validate edited values
            std::string title = titleBuffer;
            std::string author = authorBuffer;
            if (title.empty() || author.empty()) {
                editBookError = "Назва та Автор обов'язкові.";
                showEditBookError = true;
                ImGui::OpenPopup("Помилка редагування книги");
            } else if (totalCopies <= 0) {
                editBookError = "Кількість копій повинна бути мінімум 1.";
                showEditBookError = true;
                ImGui::OpenPopup("Помилка редагування книги");
            } else {
                editBook();
                showEditBookPopup = false;
                showEditBookError = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Скасувати")) {
            showEditBookPopup = false;
        }

        if (ImGui::BeginPopupModal("Помилка редагування книги", &showEditBookError, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", editBookError.c_str());
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                showEditBookError = false;
            }
            ImGui::EndPopup();
        }

        ImGui::EndPopup();
    }
}

void BookManager::addBook() {
    Book newBook(isbnBuffer, titleBuffer, authorBuffer, genreBuffer, publicationYear, totalCopies);
    books.push_back(newBook);
    dbManager.addBook(newBook);
}

void BookManager::editBook() {
    if (selectedBookIndex >= 0 && selectedBookIndex < static_cast<int>(books.size())) {
        auto oldBook = books[selectedBookIndex];
        // Preserve original ISBN (primary key). Ignore changes to ISBN in edit dialog.
        std::string origIsbn = oldBook.getISBN();

        int newTotal = totalCopies;
      int prevAvailable = oldBook.getAvailableCopies();

        int diff = newTotal - oldBook.getTotalCopies();
        int newAvailable = prevAvailable + diff;

        if (newAvailable > newTotal) newAvailable = newTotal;
        if (newAvailable < 0) newAvailable = 0; 
        Book updated(origIsbn,
                     std::string(titleBuffer),
                     std::string(authorBuffer),
                     std::string(genreBuffer),
                     publicationYear,
                     newTotal,
                     newAvailable,
                     oldBook.getStatus());

        // Persist to DB and update local cache
        if (dbManager.updateBook(updated)) {
            books[selectedBookIndex] = updated;
        } else {
            // If DB update failed, reload list to reflect DB state
            loadBooks();
        }
    }
}

void BookManager::deleteBook(int index) {
    if (index >= 0 && index < static_cast<int>(books.size())) {
        std::string isbn = books[index].getISBN();
        books.erase(books.begin() + index);
        dbManager.deleteBook(isbn);
        selectedBookIndex = -1;
    }
}

void BookManager::loadBooks() {
    books = dbManager.getAllBooks();
}

void BookManager::searchBooks(const std::string& query) {
    if (query.empty()) {
        loadBooks();
        return;
    }
    
    std::vector<Book> allBooks = dbManager.getAllBooks();
    books.clear();
    
    for (const auto& book : allBooks) {
        if (book.getTitle().find(query) != std::string::npos ||
            book.getAuthor().find(query) != std::string::npos ||
            book.getISBN().find(query) != std::string::npos) {
            books.push_back(book);
        }
    }
}
