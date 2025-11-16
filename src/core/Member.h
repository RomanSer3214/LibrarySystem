#pragma once
#include <string>

class Member {
public:
    enum class Type { STUDENT, FACULTY, EXTERNAL };
    
private:
    int id;
    std::string name;
    std::string email;
    std::string phone;
    Type type;
    int maxBooksAllowed;

public:
    Member(int id, const std::string& name, const std::string& email, const std::string& phone, Type memberType);
    Member(int id, const std::string& name, const std::string& email, const std::string& phone, Type memberType, int maxAllowed);

    int getId() const { return id; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    std::string getPhone() const { return phone; }
    Type getType() const { return type; }
    std::string getTypeString() const;
    int getMaxBooksAllowed() const { return maxBooksAllowed; }
    
    void setPhone(const std::string& p) { phone = p; }
};
