/*
 * Copyright (c) 2024 Diyor Sattarov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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

