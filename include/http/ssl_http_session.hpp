#ifndef SSL_HTTP_SESSION_HPP
#define SSL_HTTP_SESSION_HPP

#include "../util/beast.hpp"
#include "../util/util.hpp"
#include "http_session.hpp"

/**
 * @brief Class for handling SSL HTTP connections.
 * 
 * This class extends the functionality of `http_session` to handle secure
 * HTTP connections using SSL. It manages the SSL handshake, reading, writing,
 * and shutdown processes. The class is designed to be used with Boost.Beast
 * and Boost.Asio for asynchronous networking.
 */
class ssl_http_session
    : public http_session<ssl_http_session>
    , public std::enable_shared_from_this<ssl_http_session>
{
    ssl::stream<beast::tcp_stream> stream_; ///< The SSL stream used for secure communication

public:
    /**
     * @brief Constructor that initializes the session with the stream, SSL context, buffer, and document root.
     * 
     * This constructor sets up the SSL stream, initializes the session with the provided
     * buffer, and sets the document root for serving files. It also ensures that the stream
     * is moved and the SSL context is applied.
     * 
     * @param stream The TCP stream used for the connection, wrapped in an SSL stream.
     * @param ctx The SSL context containing the server certificate and key.
     * @param buffer A buffer used for reading and writing data.
     * @param doc_root A shared pointer to the document root directory.
     */
    ssl_http_session(beast::tcp_stream&& stream, ssl::context& ctx, beast::flat_buffer&& buffer, std::shared_ptr<std::string const> const& doc_root)
        : http_session<ssl_http_session>(std::move(buffer), doc_root),  // Pass buffer and doc_root to the base class
          stream_(std::move(stream), ctx)  // Initialize the SSL stream
    {
    }

    /**
     * @brief Start the SSL session.
     * 
     * This function initiates the SSL handshake and sets a timeout for the operation.
     * If the handshake is successful, the session proceeds to reading data.
     */
    void run()
    {
        // Set the timeout for the operation.
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Perform the SSL handshake. This is the buffered version of the handshake.
        stream_.async_handshake(
            ssl::stream_base::server,  // Indicate that this is a server-side handshake
            buffer_.data(),  // Use the buffer for the handshake process
            beast::bind_front_handler(
                &ssl_http_session::on_handshake,  // Handler for the handshake result
                shared_from_this()));
    }

    /**
     * @brief Get the SSL stream.
     * 
     * This function provides access to the SSL stream used in the session.
     * It is called by the base class during read and write operations.
     * 
     * @return A reference to the SSL stream.
     */
    ssl::stream<beast::tcp_stream>& stream()
    {
        return stream_;
    }

    /**
     * @brief Release the SSL stream.
     * 
     * This function moves and returns the SSL stream, transferring ownership.
     * It is typically called when the session is upgraded to a WebSocket connection.
     * 
     * @return The SSL stream, moved out of the session.
     */
    ssl::stream<beast::tcp_stream> release_stream()
    {
        return std::move(stream_);
    }

    /**
     * @brief Perform a graceful shutdown of the SSL session.
     * 
     * This function initiates an asynchronous SSL shutdown to close the connection
     * gracefully. It sets a timeout for the shutdown process and uses a handler
     * to manage the result.
     */
    void do_eof()
    {
        // Set the timeout for the shutdown operation.
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Perform the SSL shutdown
        stream_.async_shutdown(
            beast::bind_front_handler(
                &ssl_http_session::on_shutdown,  // Handler for the shutdown result
                shared_from_this()));
    }

private:
    /**
     * @brief Handle the result of the SSL handshake.
     * 
     * This function is called after the SSL handshake is completed. If the handshake
     * is successful, the session begins reading data. Otherwise, it logs the error.
     * 
     * @param ec The error code returned by the handshake operation.
     * @param bytes_used The number of bytes used from the buffer during the handshake.
     */
    void on_handshake(beast::error_code ec, std::size_t bytes_used)
    {
        if(ec)
            return fail(ec, "handshake");

        // Consume the portion of the buffer used by the handshake
        buffer_.consume(bytes_used);

        // Start reading data
        do_read();
    }

    /**
     * @brief Handle the result of the SSL shutdown.
     * 
     * This function is called after the SSL shutdown is completed. It logs any errors
     * that occurred during the shutdown process.
     * 
     * @param ec The error code returned by the shutdown operation.
     */
    void on_shutdown(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "shutdown");

        // At this point the connection is closed gracefully
    }
};

#endif

