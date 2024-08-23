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

