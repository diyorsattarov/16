#ifndef PLAIN_HTTP_SESSION_HPP
#define PLAIN_HTTP_SESSION_HPP

#include "../util/beast.hpp"
#include "../util/util.hpp"

#include "http_session.hpp"


// Handles a plain HTTP connection
class plain_http_session
    : public http_session<plain_http_session>
    , public std::enable_shared_from_this<plain_http_session>
{
    beast::tcp_stream stream_;

public:
    // Create the session
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

    // Start the session
    void
    run()
    {
        this->do_read();
    }

    // Called by the base class
    beast::tcp_stream&
    stream()
    {
        return stream_;
    }

    // Called by the base class
    beast::tcp_stream
    release_stream()
    {
        return std::move(stream_);
    }

    // Called by the base class
    void
    do_eof()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

#endif
