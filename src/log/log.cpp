#include "../../include/log/log.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>

// Static member initialization
std::mutex LoggerManager::mutex_;
std::map<std::string, std::shared_ptr<Logger>> LoggerManager::loggers_;

/**
 * @brief Constructs a Logger object.
 * 
 * Initializes the logger with a name, log level, output type, and optionally a filename for file logging.
 * If the output is set to file and a filename is provided, the log file is opened.
 * 
 * @param name The name of the logger.
 * @param level The log level (DEBUG, INFO, WARN, ERROR).
 * @param output The output type (CONSOLE, FILE).
 * @param filename The name of the log file (optional, only used if output is FILE).
 * @throws std::runtime_error if the log file cannot be opened.
 */
Logger::Logger(const std::string& name, LogLevel level, LogOutput output, const std::string& filename)
    : name_(name), level_(level), output_(output) {
    if (output_ == LogOutput::FILE && !filename.empty()) {
        file_.open(filename, std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }
}

/**
 * @brief Destructor for the Logger class.
 * 
 * Closes the log file if it is open.
 */
Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}

/**
 * @brief Logs a message at the specified log level.
 * 
 * If the message's log level is greater than or equal to the logger's current level, the message is formatted
 * and written to the specified output (console or file).
 * 
 * @param level The log level of the message.
 * @param message The message to log.
 */
void Logger::log(LogLevel level, const std::string& message) {
    if (level >= level_) {
        std::lock_guard<std::mutex> lock(mutex_);
        writeToOutput(levelToString(level) + " [" + name_ + "] " + message);
    }
}

/**
 * @brief Sets the log level for the logger.
 * 
 * Only messages with a level greater than or equal to this level will be logged.
 * 
 * @param level The new log level.
 */
void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

/**
 * @brief Sets the output type for the logger.
 * 
 * Changes the logger's output between console and file. If set to file, a filename must be provided.
 * 
 * @param output The new output type (CONSOLE, FILE).
 * @param filename The name of the log file (required if output is FILE).
 * @throws std::runtime_error if the log file cannot be opened.
 */
void Logger::setOutput(LogOutput output, const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    output_ = output;
    if (output_ == LogOutput::FILE && !filename.empty()) {
        file_.open(filename, std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }
}

/**
 * @brief Converts a log level to its string representation.
 * 
 * @param level The log level.
 * @return A string representing the log level ("DEBUG", "INFO", "WARN", "ERROR").
 */
std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Writes a formatted message to the configured output (console or file).
 * 
 * The message is prefixed with the current timestamp.
 * 
 * @param message The message to write.
 */
void Logger::writeToOutput(const std::string& message) {
    std::ostringstream oss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " - " << message;

    if (output_ == LogOutput::CONSOLE) {
        std::cout << oss.str() << std::endl;
    } else if (output_ == LogOutput::FILE && file_.is_open()) {
        file_.write(oss.str().c_str(), oss.str().length());
        file_ << std::endl;
    }
}

/**
 * @brief Operator overload to handle streaming to the logger.
 * 
 * This allows you to use the logger with stream syntax (e.g., `logger << "message";`).
 * 
 * @param os The output stream manipulator (e.g., std::endl).
 * @return A reference to the Logger object.
 */
Logger& Logger::operator<<(std::ostream& (*os)(std::ostream&)) {
    std::lock_guard<std::mutex> lock(mutex_);
    writeToOutput(buffer_.str());
    buffer_.str("");
    buffer_.clear();
    return *this;
}

/**
 * @brief Retrieves a logger instance by name.
 * 
 * If a logger with the given name does not exist, a new one is created with the specified parameters.
 * 
 * @param name The name of the logger.
 * @param level The log level (optional, defaults to INFO).
 * @param output The output type (optional, defaults to CONSOLE).
 * @param filename The name of the log file (optional, only used if output is FILE).
 * @return A shared pointer to the logger instance.
 */
std::shared_ptr<Logger> LoggerManager::getLogger(const std::string& name, LogLevel level, LogOutput output, const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (loggers_.find(name) == loggers_.end()) {
        loggers_[name] = std::make_shared<Logger>(name, level, output, filename);
    }
    return loggers_[name];
}

