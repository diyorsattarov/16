#ifndef WEBSOCKET_SESSION_FACTORY_HPP
#define WEBSOCKET_SESSION_FACTORY_HPP

#include "plain_websocket_session.hpp"
#include "ssl_websocket_session.hpp"

template<class Body, class Allocator>
void make_websocket_session(
    beast::tcp_stream stream,
    http::request<Body, http::basic_fields<Allocator>> req)
{
    std::make_shared<plain_websocket_session>(
        std::move(stream))->run(std::move(req));
}

template<class Body, class Allocator>
void make_websocket_session(
    ssl::stream<beast::tcp_stream> stream,
    http::request<Body, http::basic_fields<Allocator>> req)
{
    std::make_shared<ssl_websocket_session>(
        std::move(stream))->run(std::move(req));
}

#endif // WEBSOCKET_SESSION_FACTORY_HPP

