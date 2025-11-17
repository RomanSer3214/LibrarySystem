#include "Member.h"

Member::Member(int id, const std::string& name, const std::string& email, const std::string& phone, Type memberType)
    : id(id), name(name), email(email), phone(phone), type(memberType) {
        switch (type) {
            case Type::STUDENT:
                maxBooksAllowed = 10;
                break;
            case Type::FACULTY:
                maxBooksAllowed = 15;
                break;
            case Type::EXTERNAL:
                maxBooksAllowed = 3;
                break;
            default:
                maxBooksAllowed = 0;
                break;
        }

    } // default

Member::Member(int id, const std::string& name, const std::string& email, const std::string& phone, Type memberType, int maxAllowed)
    : id(id), name(name), email(email), phone(phone), type(memberType), maxBooksAllowed(maxAllowed) {}

std::string Member::getTypeString() const {
    switch (type) {
        case Type::STUDENT: return "Студент";
        case Type::FACULTY: return "Викладач";
        case Type::EXTERNAL: return "Зовнішній";
        default: return "Невідомий";
    }
}
