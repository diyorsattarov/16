#ifndef SSL_HTTP_SESSION_HPP
#define SSL_HTTP_SESSION_HPP

#include "../util/beast.hpp"
#include "../util/util.hpp"
#include "http_session.hpp"

class ssl_http_session
    : public http_session<ssl_http_session>
    , public std::enable_shared_from_this<ssl_http_session>
{
    ssl::stream<beast::tcp_stream> stream_;

public:
    // Constructor that initializes the session with the stream, SSL context, buffer, and document root
    ssl_http_session(beast::tcp_stream&& stream, ssl::context& ctx, beast::flat_buffer&& buffer, std::shared_ptr<std::string const> const& doc_root)
        : http_session<ssl_http_session>(std::move(buffer), doc_root),  // Pass buffer and doc_root to the base class
          stream_(std::move(stream), ctx)  // Initialize the SSL stream
    {
    }

    // Start the session
    void run()
    {
        // Set the timeout.
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Perform the SSL handshake
        stream_.async_handshake(
            ssl::stream_base::server,
            buffer_.data(),
            beast::bind_front_handler(
                &ssl_http_session::on_handshake,
                shared_from_this()));
    }

    // Called by the base class to get the stream
    ssl::stream<beast::tcp_stream>& stream()
    {
        return stream_;
    }

    // Called by the base class to release the stream
    ssl::stream<beast::tcp_stream> release_stream()
    {
        return std::move(stream_);
    }

    // Called by the base class to perform a graceful shutdown
    void do_eof()
    {
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        stream_.async_shutdown(
            beast::bind_front_handler(
                &ssl_http_session::on_shutdown,
                shared_from_this()));
    }

private:
    // Handle the result of the SSL handshake
    void on_handshake(beast::error_code ec, std::size_t bytes_used)
    {
        if(ec)
            return fail(ec, "handshake");

        // Consume the portion of the buffer used by the handshake
        buffer_.consume(bytes_used);

        do_read();
    }

    // Handle the result of the SSL shutdown
    void on_shutdown(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "shutdown");

        // At this point the connection is closed gracefully
    }
};

#endif

