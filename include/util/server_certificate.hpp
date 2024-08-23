#ifndef SERVER_CERTIFICATE_HPP
#define SERVER_CERTIFICATE_HPP

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <string>
#include "dotenv.hpp"
#include "../log/log.hpp"
/**
 * @brief Load the content of a file into a string.
 * 
 * @param file_path The path to the file to be loaded.
 * @return A string containing the file's content.
 * @throws std::runtime_error if the file cannot be opened.
 */
std::string load_file_content(const std::string& file_path);

/**
 * @brief Load the server certificate, private key, and DH parameters into the SSL context.
 * 
 * The function reads the necessary file paths and password from environment variables,
 * loads the files' content, and configures the SSL context.
 * 
 * @param ctx The SSL context to configure.
 * @throws std::runtime_error if required environment variables are missing or if file loading fails.
 */
void load_server_certificate(boost::asio::ssl::context& ctx);

#endif // SERVER_CERTIFICATE_HPP

