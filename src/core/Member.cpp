#include "Member.h"

Member::Member(int id, const std::string& name, const std::string& email, Type memberType)
    : id(id), name(name), email(email), type(memberType) {
    switch(type) {
        case Type::STUDENT: maxBooksAllowed = 5; break;
        case Type::FACULTY: maxBooksAllowed = 10; break;
        case Type::EXTERNAL: maxBooksAllowed = 3; break;
    }
}

std::string Member::getTypeString() const {
    switch(type) {
        case Type::STUDENT: return "Студент";
        case Type::FACULTY: return "Викладач";
        case Type::EXTERNAL: return "Зовнішній";
        default: return "Невідомий";
    }
}
