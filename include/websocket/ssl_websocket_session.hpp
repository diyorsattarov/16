#ifndef SSL_WEBSOCKET_SESSION_HPP
#define SSL_WEBSOCKET_SESSION_HPP

#include "../../include/util/beast.hpp"
#include "websocket_session.hpp"

class ssl_websocket_session
    : public websocket_session<ssl_websocket_session>
    , public std::enable_shared_from_this<ssl_websocket_session>
{
    websocket::stream<ssl::stream<beast::tcp_stream>> ws_;

public:
    explicit ssl_websocket_session(ssl::stream<beast::tcp_stream>&& stream)
        : ws_(std::move(stream))
    {
    }

    websocket::stream<ssl::stream<beast::tcp_stream>>& ws()
    {
        return ws_;
    }
};

#endif
