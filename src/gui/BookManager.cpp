#include "BookManager.h"
#include <algorithm>
#include <cstring>

BookManager::BookManager(DatabaseManager& db) : dbManager(db) {
    loadBooks();
}

void BookManager::render() {
    renderSearchBar();
    
    ImGui::Separator();
    
    if (ImGui::Button("Додати книгу")) {
        showAddBookPopup = true;
        memset(isbnBuffer, 0, sizeof(isbnBuffer));
        memset(titleBuffer, 0, sizeof(titleBuffer));
        memset(authorBuffer, 0, sizeof(authorBuffer));
        memset(publisherBuffer, 0, sizeof(publisherBuffer));
        publicationYear = 2024;
        totalCopies = 1;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Редагувати") && selectedBookIndex >= 0) {
        showEditBookPopup = true;
        const auto& book = books[selectedBookIndex];
        
        strncpy(isbnBuffer, book.getISBN().c_str(), sizeof(isbnBuffer) - 1);
        strncpy(titleBuffer, book.getTitle().c_str(), sizeof(titleBuffer) - 1);
        strncpy(authorBuffer, book.getAuthor().c_str(), sizeof(authorBuffer) - 1);
        strncpy(publisherBuffer, book.getPublisher().c_str(), sizeof(publisherBuffer) - 1);
        
        isbnBuffer[sizeof(isbnBuffer) - 1] = '\0';
        titleBuffer[sizeof(titleBuffer) - 1] = '\0';
        authorBuffer[sizeof(authorBuffer) - 1] = '\0';
        publisherBuffer[sizeof(publisherBuffer) - 1] = '\0';
        
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
    ImGui::InputText("Пошук", searchBuffer, sizeof(searchBuffer));
    if (ImGui::Button("Шукати")) {
        searchBooks(searchBuffer);
    }
    ImGui::SameLine();
    if (ImGui::Button("Очистити")) {
        memset(searchBuffer, 0, sizeof(searchBuffer));
        loadBooks();
    }
}

void BookManager::renderBookList() {
    if (ImGui::BeginTable("BooksTable", 6, 
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable)) {
        
        ImGui::TableSetupColumn("ISBN");
        ImGui::TableSetupColumn("Назва");
        ImGui::TableSetupColumn("Автор");
        ImGui::TableSetupColumn("Рік");
        ImGui::TableSetupColumn("Доступно");
        ImGui::TableSetupColumn("Статус");
        ImGui::TableHeadersRow();
        
        for (int i = 0; i < books.size(); i++) {
            const auto& book = books[i];
            ImGui::TableNextRow();
            
            if (i == selectedBookIndex) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 
                    ImGui::GetColorU32(ImGuiCol_Header));
            }
            
            if (ImGui::IsItemClicked()) {
                selectedBookIndex = i;
            }
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", book.getISBN().c_str());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", book.getTitle().c_str());
            
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", book.getAuthor().c_str());
            
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", book.getPublicationYear());
            
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%d/%d", book.getAvailableCopies(), book.getTotalCopies());
            
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", book.getStatusString().c_str());
        }
        
        ImGui::EndTable();
    }
}

void BookManager::renderAddBookPopup() {
    ImGui::OpenPopup("Додати книгу");
    if (ImGui::BeginPopupModal("Додати книгу", &showAddBookPopup)) {
        ImGui::InputText("ISBN", isbnBuffer, sizeof(isbnBuffer));
        ImGui::InputText("Назва", titleBuffer, sizeof(titleBuffer));
        ImGui::InputText("Автор", authorBuffer, sizeof(authorBuffer));
        ImGui::InputText("Видавництво", publisherBuffer, sizeof(publisherBuffer));
        ImGui::InputInt("Рік публікації", &publicationYear);
        ImGui::InputInt("Кількість копій", &totalCopies);
        
        if (ImGui::Button("Зберегти")) {
            addBook();
            showAddBookPopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Скасувати")) {
            showAddBookPopup = false;
        }
        
        ImGui::EndPopup();
    }
}

void BookManager::renderEditBookPopup() {
    ImGui::OpenPopup("Редагувати книгу");
    if (ImGui::BeginPopupModal("Редагувати книгу", &showEditBookPopup)) {
        ImGui::InputText("ISBN", isbnBuffer, sizeof(isbnBuffer));
        ImGui::InputText("Назва", titleBuffer, sizeof(titleBuffer));
        ImGui::InputText("Автор", authorBuffer, sizeof(authorBuffer));
        ImGui::InputText("Видавництво", publisherBuffer, sizeof(publisherBuffer));
        ImGui::InputInt("Рік публікації", &publicationYear);
        ImGui::InputInt("Кількість копій", &totalCopies);
        
        if (ImGui::Button("Зберегти")) {
            editBook();
            showEditBookPopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Скасувати")) {
            showEditBookPopup = false;
        }
        
        ImGui::EndPopup();
    }
}

void BookManager::addBook() {
    Book newBook(isbnBuffer, titleBuffer, authorBuffer, publisherBuffer, publicationYear, totalCopies);
    books.push_back(newBook);
    dbManager.addBook(newBook);
}

void BookManager::editBook() {
    if (selectedBookIndex >= 0 && selectedBookIndex < books.size()) {
        auto& book = books[selectedBookIndex];
        // Оновлення книги
        dbManager.updateBook(book);
    }
}

void BookManager::deleteBook(int index) {
    if (index >= 0 && index < books.size()) {
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
    
    // Простий пошук (в реальному проекті краще використовувати SQL LIKE)
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
