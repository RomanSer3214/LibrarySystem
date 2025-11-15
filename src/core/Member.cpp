#include "Member.h"

Member::Member(int id, const std::string& name, const std::string& email, const std::string& phone, Type memberType)
    : id(id), name(name), email(email), phone(phone), type(memberType), maxBooksAllowed(5) {} // default

Member::Member(int id, const std::string& name, const std::string& email, const std::string& phone, Type memberType, int maxAllowed)
    : id(id), name(name), email(email), phone(phone), type(memberType), maxBooksAllowed(maxAllowed) {}

std::string Member::getTypeString() const {
    switch (type) {
        case Type::STUDENT: return "Student";
        case Type::FACULTY: return "Faculty";
        case Type::EXTERNAL: return "External";
        default: return "Unknown";
    }
}
