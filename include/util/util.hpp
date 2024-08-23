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

#ifndef UTILS_HPP
#define UTILS_HPP

#include "beast.hpp"
#include <iostream>
#include <string>

/**
 * @brief Handles and logs errors that occur during asynchronous operations.
 * 
 * This function logs an error message if the error code is not related to a truncated SSL stream.
 * It is intended to be used as a callback in asynchronous operations where error handling is required.
 * 
 * @param ec The error code object that contains information about the error.
 * @param what A description of the operation or context in which the error occurred.
 */
inline void fail(boost::beast::error_code ec, char const* what)
{
    // Ignore SSL stream truncation errors, as they are expected during connection termination.
    if(ec == net::ssl::error::stream_truncated)
        return;

    // Log the error message to standard error output.
    std::cerr << what << ": " << ec.message() << "\n";
}

#endif // UTILS_HPP

