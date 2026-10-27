#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

void Logger::Log(LogLevel level, const std::string& sender, const std::string& message) {
    const char* prefix = "[INFO]";
    const char* color = "\033[0m";

    switch (level) {
        case LogLevel::DEBUG:
            prefix = "[DEBUG]";
            color = "\033[36m"; // Cyan
            break;
        case LogLevel::INFO:
            prefix = "[INFO] ";
            color = "\033[32m"; // Green
            break;
        case LogLevel::WARN:
            prefix = "[WARN] ";
            color = "\033[33m"; // Yellow
            break;
        case LogLevel::ERR:
            prefix = "[ERROR]";
            color = "\033[31m"; // Red
            break;
        case LogLevel::COMBAT:
            prefix = "[FIGHT]";
            color = "\033[35m"; // Magenta / Tekken purple
            break;
    }

    std::cout << color << prefix << " [" << sender << "] " << message << "\033[0m" << std::endl;
}

} // namespace DarkLotus
