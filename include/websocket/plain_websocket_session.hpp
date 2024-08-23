#ifndef PLAIN_WEBSOCKET_SESSION_HPP
#define PLAIN_WEBSOCKET_SESSION_HPP

#include "../../include/util/beast.hpp"
#include "websocket_session.hpp"

class plain_websocket_session
    : public websocket_session<plain_websocket_session>
    , public std::enable_shared_from_this<plain_websocket_session>
{
    websocket::stream<beast::tcp_stream> ws_;

public:
    explicit plain_websocket_session(beast::tcp_stream&& stream)
        : ws_(std::move(stream))
    {
    }

    websocket::stream<beast::tcp_stream>& ws()
    {
        return ws_;
    }
};

#endif
