#ifndef PLAIN_WEBSOCKET_SESSION_HPP
#define PLAIN_WEBSOCKET_SESSION_HPP

#include "../../include/util/beast.hpp"
#include "websocket_session.hpp"

/**
 * @brief Class for handling plain (non-SSL) WebSocket connections.
 * 
 * This class extends the `websocket_session` template to manage a plain WebSocket
 * session without SSL. It manages the WebSocket stream, including reading, writing,
 * and other WebSocket-specific operations.
 */
class plain_websocket_session
: public websocket_session<plain_websocket_session>
    , public std::enable_shared_from_this<plain_websocket_session>
{
    websocket::stream<beast::tcp_stream> ws_; ///< The WebSocket stream for plain (non-SSL) connections.

    public:
    /**
     * @brief Constructor that initializes the WebSocket session with a TCP stream.
     * 
     * This constructor moves the provided TCP stream into the WebSocket stream, which
     * will be used to manage WebSocket-specific operations such as reading and writing messages.
     * 
     * @param stream The TCP stream used for the WebSocket connection.
     */
    explicit plain_websocket_session(beast::tcp_stream&& stream)
        : ws_(std::move(stream))  // Move the TCP stream into the WebSocket stream
    {
    }

    /**
     * @brief Get the WebSocket stream.
     * 
     * This function provides access to the underlying WebSocket stream, which is used
     * to send and receive WebSocket frames.
     * 
     * @return A reference to the WebSocket stream.
     */
    websocket::stream<beast::tcp_stream>& ws()
    {
        return ws_;
    }
};

#endif

