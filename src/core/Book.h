#pragma once
#include <string>

class Book {
public:
    enum class Status { AVAILABLE, BORROWED, RESERVED, MAINTENANCE };
    
private:
    std::string isbn;
    std::string title;
    std::string author;
    std::string genre;
    int publicationYear;
    int totalCopies; 
    int availableCopies;
    Status status;

public:
    Book(const std::string& isbn, const std::string& title, const std::string& author, const std::string& genre, int publicationYear, int copies);
    Book(const std::string& isbn, const std::string& title, const std::string& author, const std::string& genre, int publicationYear, int totalCopies, int availableCopies, Status status);
    
    std::string getISBN() const { return isbn; }
    std::string getTitle() const { return title; }
    std::string getAuthor() const { return author; }
    std::string getGenre() const { return genre; }
    int getPublicationYear() const { return publicationYear; }
    int getTotalCopies() const { return totalCopies; }
    int getAvailableCopies() const { return availableCopies; }
    Status getStatus() const { return status; }
    std::string getStatusString() const;
    
    bool borrowBook();
    bool returnBook();
    bool reserveBook();
    bool isAvailable() const;
    
    void setTotalCopies(int copies);
};
