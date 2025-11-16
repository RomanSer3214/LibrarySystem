#pragma once
#include <string>

class Book {
public:
    enum class Status { AVAILABLE = 0, BORROWED = 1, RESERVED = 2, MAINTENANCE = 3 };

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
    // Constructors
    Book(const std::string& isbn, const std::string& title, const std::string& author,
         const std::string& genre, int publicationYear, int copies);
    Book(const std::string& isbn, const std::string& title, const std::string& author,
         const std::string& genre, int publicationYear, int totalCopies, int availableCopies, Status status);

    std::string getISBN() const { return isbn; }
    std::string getTitle() const { return title; }
    std::string getAuthor() const { return author; }
    std::string getGenre() const { return genre; }
    int getPublicationYear() const { return publicationYear; }
    int getTotalCopies() const { return totalCopies; }
    int getAvailableCopies() const { return availableCopies; }
    Status getStatus() const { return status; }

    void setAvailableCopies(int copies) { availableCopies = copies < 0 ? 0 : copies; if (availableCopies > totalCopies) availableCopies = totalCopies; }
    void setTotalCopies(int copies) { totalCopies = copies < 0 ? 0 : copies; if (availableCopies > totalCopies) availableCopies = totalCopies; }
    void setStatus(Status s) { status = s; }

    std::string getStatusString() const;
    bool borrowBook();
    bool returnBook();   
    bool reserveBook();
    bool isAvailable() const;
};
