#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace DarkLotus {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERR,
    COMBAT
};

class Logger {
public:
    static void Log(LogLevel level, const std::string& sender, const std::string& message);
    static void Info(const std::string& sender, const std::string& message) {
        Log(LogLevel::INFO, sender, message);
    }
    static void Warn(const std::string& sender, const std::string& message) {
        Log(LogLevel::WARN, sender, message);
    }
    static void Error(const std::string& sender, const std::string& message) {
        Log(LogLevel::ERR, sender, message);
    }
    static void Combat(const std::string& sender, const std::string& message) {
        Log(LogLevel::COMBAT, sender, message);
    }
};

} // namespace DarkLotus
