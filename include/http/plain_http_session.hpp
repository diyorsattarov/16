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

#ifndef PLAIN_HTTP_SESSION_HPP
#define PLAIN_HTTP_SESSION_HPP

#include "../util/beast.hpp"
#include "../util/util.hpp"
#include "http_session.hpp"

/**
 * @brief The plain_http_session class handles a plain (non-SSL) HTTP connection.
 * 
 * This class is responsible for managing the lifecycle of a plain HTTP connection.
 * It reads HTTP requests from the client, processes them, and sends back appropriate responses.
 * The class is templated to allow for reuse with different derived session types.
 */
class plain_http_session
: public http_session<plain_http_session>
    , public std::enable_shared_from_this<plain_http_session>
{
    beast::tcp_stream stream_; ///< The TCP stream used for communication with the client.

    public:
    /**
     * @brief Constructor for the plain_http_session class.
     * 
     * This constructor initializes the plain_http_session with the provided TCP stream, buffer, and document root.
     * 
     * @param stream The TCP stream for the session.
     * @param buffer A buffer used to hold incoming data.
     * @param doc_root The root directory from which to serve files.
     */
    plain_http_session(
            beast::tcp_stream&& stream,
            beast::flat_buffer&& buffer,
            std::shared_ptr<std::string const> const& doc_root)
        : http_session<plain_http_session>(
                std::move(buffer),
                doc_root)
          , stream_(std::move(stream))
    {
    }

    /**
     * @brief Start the session.
     * 
     * This method begins the process of reading HTTP requests from the client.
     */
    void run()
    {
        this->do_read();
    }

    /**
     * @brief Access the underlying TCP stream.
     * 
     * This method is called by the base class to access the TCP stream.
     * 
     * @return A reference to the TCP stream.
     */
    beast::tcp_stream& stream()
    {
        return stream_;
    }

    /**
     * @brief Release ownership of the TCP stream.
     * 
     * This method is called by the base class when ownership of the TCP stream needs to be transferred,
     * for example, when upgrading to a WebSocket session.
     * 
     * @return The TCP stream, with ownership transferred to the caller.
     */
    beast::tcp_stream release_stream()
    {
        return std::move(stream_);
    }

    /**
     * @brief Perform a graceful shutdown of the TCP connection.
     * 
     * This method is called by the base class to shut down the TCP connection when the session ends.
     */
    void do_eof()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

#endif // PLAIN_HTTP_SESSION_HPP

