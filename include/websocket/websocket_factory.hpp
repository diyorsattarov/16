#ifndef WEBSOCKET_SESSION_FACTORY_HPP
#define WEBSOCKET_SESSION_FACTORY_HPP

#include "plain_websocket_session.hpp"
#include "ssl_websocket_session.hpp"

/**
 * @brief Create and run a plain WebSocket session.
 * 
 * This template function takes a TCP stream and an HTTP request, creates a plain (non-SSL)
 * WebSocket session, and runs it. The function is designed to be used with non-secure WebSocket
 * connections.
 * 
 * @tparam Body The type of the HTTP request body.
 * @tparam Allocator The type of the allocator used in the HTTP request.
 * @param stream The TCP stream used for the WebSocket connection.
 * @param req The HTTP request to be handled by the WebSocket session.
 */
template<class Body, class Allocator>
void make_websocket_session(
    beast::tcp_stream stream,
    http::request<Body, http::basic_fields<Allocator>> req)
{
    // Create a plain WebSocket session and run it with the given request
    std::make_shared<plain_websocket_session>(
        std::move(stream))->run(std::move(req));
}

/**
 * @brief Create and run an SSL WebSocket session.
 * 
 * This template function takes an SSL stream and an HTTP request, creates an SSL
 * WebSocket session, and runs it. The function is designed to be used with secure
 * WebSocket connections over SSL/TLS.
 * 
 * @tparam Body The type of the HTTP request body.
 * @tparam Allocator The type of the allocator used in the HTTP request.
 * @param stream The SSL stream used for the WebSocket connection.
 * @param req The HTTP request to be handled by the WebSocket session.
 */
template<class Body, class Allocator>
void make_websocket_session(
    ssl::stream<beast::tcp_stream> stream,
    http::request<Body, http::basic_fields<Allocator>> req)
{
    // Create an SSL WebSocket session and run it with the given request
    std::make_shared<ssl_websocket_session>(
        std::move(stream))->run(std::move(req));
}

#endif // WEBSOCKET_SESSION_FACTORY_HPP

