#include "DateUtils.h"
#include <sstream>
#include <iomanip>

std::string DateUtils::timePointToString(const std::chrono::system_clock::time_point& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&time_t);
    
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string DateUtils::getCurrentDateTime() {
    return timePointToString(std::chrono::system_clock::now());
}

int DateUtils::daysBetween(const std::chrono::system_clock::time_point& from, 
                          const std::chrono::system_clock::time_point& to) {
    auto duration = to - from;
    return std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24;
}
