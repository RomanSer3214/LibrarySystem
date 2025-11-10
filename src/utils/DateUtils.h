#pragma once
#include <string>
#include <chrono>

class DateUtils {
public:
    static std::string timePointToString(const std::chrono::system_clock::time_point& tp);
    static std::chrono::system_clock::time_point stringToTimePoint(const std::string& str);
    static std::string getCurrentDateTime();
    static int daysBetween(const std::chrono::system_clock::time_point& from, 
                          const std::chrono::system_clock::time_point& to);
};
