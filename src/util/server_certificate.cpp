#include "../../include/util/server_certificate.hpp"
#include "../../include/log/log.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

/**
 * @brief Load the content of a file into a string.
 * 
 * This function reads the entire content of a file specified by `file_path` and returns it as a string.
 * 
 * @param file_path The path to the file to be loaded.
 * @return A string containing the file's content.
 * @throws std::runtime_error if the file cannot be opened.
 */
std::string load_file_content(const std::string& file_path) {
    auto logger = LoggerManager::getLogger("server_certificate_logger", LogLevel::INFO);
    logger->log(LogLevel::DEBUG, "Loading file content from: " + file_path);

    std::ifstream file(file_path);
    if (!file.is_open()) {
        logger->log(LogLevel::ERROR, "Error opening file: " + file_path);
        throw std::runtime_error("Could not open file: " + file_path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    logger->log(LogLevel::DEBUG, "File content loaded successfully.");
    return buffer.str();
}

/**
 * @brief Load the server certificate, private key, and DH parameters into the SSL context.
 * 
 * This function reads the necessary file paths and password from environment variables,
 * loads the files' content, and configures the SSL context accordingly.
 * 
 * @param ctx The SSL context to configure.
 * @throws std::runtime_error if required environment variables are missing or if file loading fails.
 */
void load_server_certificate(boost::asio::ssl::context& ctx)
{
    auto logger = LoggerManager::getLogger("server_certificate_logger", LogLevel::INFO);
    logger->log(LogLevel::DEBUG, "Loading server certificate.");

    // Load environment variables from the .env file
    dotenv::init(".env");
    logger->log(LogLevel::DEBUG, "Environment variables loaded.");

    // Retrieve file paths and password from the environment
    const char* cert_path = std::getenv("CERT_PATH");
    const char* key_path = std::getenv("KEY_PATH");
    const char* dh_path = std::getenv("DH_PATH");
    const char* password_cstr = std::getenv("SSL_PASSWORD");

    // Ensure all required environment variables are set
    if (!cert_path || !key_path || !dh_path || !password_cstr) {
        logger->log(LogLevel::ERROR, "Missing one or more required environment variables.");
        throw std::runtime_error("Missing one or more required environment variables");
    }

    logger->log(LogLevel::DEBUG, "Environment variables found.");

    // Load the contents of the certificate, key, and DH parameter files
    std::string cert = load_file_content(cert_path);
    std::string key = load_file_content(key_path);
    std::string dh = load_file_content(dh_path);
    std::string password(password_cstr);

    logger->log(LogLevel::DEBUG, "Setting SSL context password callback.");
    ctx.set_password_callback(
        [password](std::size_t,
                   boost::asio::ssl::context_base::password_purpose)
        {
            return password;
        });

    logger->log(LogLevel::DEBUG, "Configuring SSL context options.");
    ctx.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::single_dh_use);

    logger->log(LogLevel::DEBUG, "Loading certificate chain.");
    ctx.use_certificate_chain(
        boost::asio::buffer(cert.data(), cert.size()));

    logger->log(LogLevel::DEBUG, "Loading private key.");
    ctx.use_private_key(
        boost::asio::buffer(key.data(), key.size()),
        boost::asio::ssl::context::file_format::pem);

    logger->log(LogLevel::DEBUG, "Loading DH parameters.");
    ctx.use_tmp_dh(
        boost::asio::buffer(dh.data(), dh.size()));

    logger->log(LogLevel::DEBUG, "Server certificate loaded successfully.");
}

