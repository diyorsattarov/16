#ifndef SSL_WEBSOCKET_SESSION_HPP
#define SSL_WEBSOCKET_SESSION_HPP

#include "../../include/util/beast.hpp"
#include "websocket_session.hpp"

/**
 * @brief Class for handling SSL WebSocket connections.
 * 
 * This class extends the `websocket_session` template to manage a WebSocket session
 * over an SSL/TLS connection. It manages the WebSocket stream, including reading, writing,
 * and other WebSocket-specific operations while ensuring secure communication.
 */
class ssl_websocket_session
    : public websocket_session<ssl_websocket_session>
    , public std::enable_shared_from_this<ssl_websocket_session>
{
    websocket::stream<ssl::stream<beast::tcp_stream>> ws_; ///< The WebSocket stream for SSL/TLS connections.

public:
    /**
     * @brief Constructor that initializes the SSL WebSocket session with a TCP stream.
     * 
     * This constructor moves the provided SSL stream into the WebSocket stream, which
     * will be used to manage WebSocket-specific operations such as reading and writing messages
     * over a secure SSL/TLS connection.
     * 
     * @param stream The SSL stream used for the WebSocket connection.
     */
    explicit ssl_websocket_session(ssl::stream<beast::tcp_stream>&& stream)
        : ws_(std::move(stream))  // Move the SSL stream into the WebSocket stream
    {
    }

    /**
     * @brief Get the WebSocket stream.
     * 
     * This function provides access to the underlying WebSocket stream, which is used
     * to send and receive WebSocket frames over an SSL/TLS connection.
     * 
     * @return A reference to the WebSocket stream.
     */
    websocket::stream<ssl::stream<beast::tcp_stream>>& ws()
    {
        return ws_;
    }
};

#endif

