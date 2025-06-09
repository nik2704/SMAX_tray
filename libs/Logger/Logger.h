#pragma once

#include <algorithm>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip> // For std::put_time
#include <ctime>   // For std::localtime
#include <mutex>   // For thread-safety

namespace AppLogger {

// Define log levels for better categorization of messages
enum LogLevel {
    LOG_DEBUG,    // Detailed information, typically only of interest to a developer.
    LOG_INFO,     // Standard events, good for general application flow.
    LOG_WARNING,  // An event that might indicate a problem.
    LOG_ERROR,    // An error event that might still allow the application to continue.
    LOG_CRITICAL  // A severe error event that will likely cause the application to abort.
};

class Logger {
public:
    // Destructor to ensure the log file is closed when the logger object is destroyed.
    ~Logger();

    // Singleton pattern: provides a global access point to the single Logger instance.
    static Logger& getInstance();

    // Set the path for the log file. Call this once at application startup.
    void setLogFile(const std::string& filename);

    // Set the minimum log level to filter messages (e.g., only log INFO and above).
    void setMinLogLevel(LogLevel level);
    void setMinLogLevel(const std::string& level);

    // Main logging method: writes a message with a specific level.
    void log(LogLevel level, const std::string& message);

private:
    // Private constructor to enforce singleton pattern.
    Logger();

    // Delete copy constructor and assignment operator to prevent copying.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream log_file_;      // The output file stream for logging.
    std::string log_filename_;    // Stores the current log file path.
    LogLevel min_log_level_;      // The minimum level of messages to be logged.
    std::mutex mutex_;            // Mutex for thread-safe file writing.

    // Helper functions
    std::string getLogLevelString(LogLevel level) const; // Converts enum to string.
    std::string getCurrentTimestamp() const;             // Generates current timestamp.
    void openLogFile();                                  // Opens the log file.
};

} // namespace AppLogger
