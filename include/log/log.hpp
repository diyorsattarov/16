#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <mutex>
#include <map>
#include <sstream>

/// Enum representing the log level severity
enum class LogLevel {
    DEBUG,  ///< Detailed debug information
    INFO,   ///< General information about application events
    WARN,   ///< Warnings about potential issues
    ERROR   ///< Errors that require attention
};

/// Enum representing the log output destination
enum class LogOutput {
    CONSOLE, ///< Log to console
    FILE     ///< Log to a file
};

/// Logger class for managing log messages with different levels and outputs
class Logger {
public:
    /**
     * @brief Constructor for the Logger class.
     * @param name The name of the logger.
     * @param level The initial logging level (default: INFO).
     * @param output The output destination (default: CONSOLE).
     * @param filename The filename for file output (optional).
     */
    Logger(const std::string& name, LogLevel level = LogLevel::INFO, LogOutput output = LogOutput::CONSOLE, const std::string& filename = "");

    /**
     * @brief Destructor for the Logger class.
     */
    ~Logger();

    /**
     * @brief Logs a message at the specified log level.
     * @param level The level of the log message.
     * @param message The log message to record.
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Sets the logging level.
     * @param level The log level to set.
     */
    void setLevel(LogLevel level);

    /**
     * @brief Sets the log output destination.
     * @param output The output destination (CONSOLE or FILE).
     * @param filename The filename for file output (optional).
     */
    void setOutput(LogOutput output, const std::string& filename = "");
    
    /**
     * @brief Operator overload for streaming messages into the logger.
     * @param message The message to log.
     * @return A reference to the Logger instance.
     */
    template<typename T>
    Logger& operator<<(const T& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_ << message;
        return *this;
    }

    /**
     * @brief Operator overload for handling stream manipulators (like std::endl).
     * @param os The stream manipulator.
     * @return A reference to the Logger instance.
     */
    Logger& operator<<(std::ostream& (*os)(std::ostream&));

private:
    std::string name_;                ///< The name of the logger
    LogLevel level_;                  ///< Current log level
    LogOutput output_;                ///< Current output destination
    std::ofstream file_;              ///< File stream for file logging
    std::mutex mutex_;                ///< Mutex to protect shared resources
    std::ostringstream buffer_;       ///< Buffer for building log messages

    /**
     * @brief Converts a LogLevel enum to its corresponding string representation.
     * @param level The log level to convert.
     * @return The string representation of the log level.
     */
    std::string levelToString(LogLevel level);

    /**
     * @brief Writes the given message to the configured output destination.
     * @param message The message to log.
     */
    void writeToOutput(const std::string& message);
};

/// LoggerManager class for managing multiple Logger instances
class LoggerManager {
public:
    /**
     * @brief Retrieves or creates a logger instance by name.
     * @param name The name of the logger.
     * @param level The initial logging level (default: INFO).
     * @param output The output destination (default: CONSOLE).
     * @param filename The filename for file output (optional).
     * @return A shared pointer to the Logger instance.
     */
    static std::shared_ptr<Logger> getLogger(const std::string& name, LogLevel level = LogLevel::INFO, LogOutput output = LogOutput::CONSOLE, const std::string& filename = "");

private:
    static std::mutex mutex_;                                           ///< Mutex to protect logger map
    static std::map<std::string, std::shared_ptr<Logger>> loggers_;    ///< Map of loggers by name
};

#endif // LOG_HPP

