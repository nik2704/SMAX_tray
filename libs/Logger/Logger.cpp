#include "Logger.h"
#include <iostream>
#include <sstream>

namespace AppLogger {

Logger::Logger() : min_log_level_(LOG_INFO) {
    // Constructor: logger created, but file not yet opened.
    // It's recommended to call setLogFile() explicitly before first log.
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

Logger& Logger::getInstance() {
    // Static instance is initialized on first use and guaranteed to be destroyed.
    static Logger instance;
    return instance;
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_); // Protect file operations
    if (log_file_.is_open()) {
        if (log_filename_ == filename) {
            return; // Already open with the same name
        }
        log_file_.close(); // Close existing file if changing path
    }
    log_filename_ = filename;
    openLogFile(); // Attempt to open the new file
}

void Logger::setMinLogLevel(LogLevel level) {
    min_log_level_ = level;
}

void Logger::setMinLogLevel(const std::string& level) {
    std::string lvl = level;
    std::transform(lvl.begin(), lvl.end(), lvl.begin(), ::toupper);

    if (lvl == "DEBUG") setMinLogLevel(LOG_DEBUG);
    if (lvl == "INFO") setMinLogLevel(LOG_INFO);
    if (lvl == "WARNING") setMinLogLevel(LOG_WARNING);
    if (lvl == "ERROR") setMinLogLevel(LOG_ERROR);
    if (lvl == "CRITICAL") setMinLogLevel(LOG_CRITICAL);
}

void Logger::openLogFile() {
    // Open in append mode (std::ios_base::app) so new logs are added to the end.
    log_file_.open(log_filename_, std::ios_base::app);
    if (!log_file_.is_open()) {
        std::cerr << "Error: Could not open log file: " << log_filename_ << std::endl;
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < min_log_level_) {
        return; // Filter out messages below the minimum configured level
    }

    std::lock_guard<std::mutex> lock(mutex_); // Ensure only one thread writes at a time

    if (!log_file_.is_open()) {
        openLogFile(); // Attempt to open if not already (e.g., first log call)
        if (!log_file_.is_open()) {
            std::cerr << "Error: Log file not open, cannot write log message." << std::endl;
            return; // Cannot write log if file isn't open
        }
    }

    log_file_ << "[" << getCurrentTimestamp() << "] "
              << "[" << getLogLevelString(level) << "] "
              << message << std::endl;

    // Flush the stream immediately for critical errors to ensure they are written.
    if (level >= LOG_ERROR) {
        log_file_.flush();
    }
}

std::string Logger::getLogLevelString(LogLevel level) const {
    switch (level) {
        case LOG_DEBUG:    return "DEBUG";
        case LOG_INFO:     return "INFO";
        case LOG_WARNING:  return "WARNING";
        case LOG_ERROR:    return "ERROR";
        case LOG_CRITICAL: return "CRITICAL";
        default:       return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm_buf;
    localtime_s(&tm_buf, &in_time_t); // безопасный вариант

    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace AppLogger