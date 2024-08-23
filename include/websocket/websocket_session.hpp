#ifndef WEBSOCKET_SESSION_HPP
#define WEBSOCKET_SESSION_HPP

#include "../util/util.hpp"
#include "../util/beast.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <queue>

/**
 * @brief Base class for managing a WebSocket session.
 * 
 * This template class provides common functionality for handling WebSocket sessions,
 * including reading from and writing to the WebSocket stream, managing buffer data,
 * and handling WebSocket-specific operations such as accepting connections and
 * sending responses. The class is designed to be used as a base class, with the
 * derived class specifying the exact type of WebSocket session (plain or SSL).
 * 
 * @tparam Derived The derived class that implements specific WebSocket session behavior.
 */
template<class Derived>
class websocket_session
{
    /**
     * @brief Get a reference to the derived class.
     * 
     * This method returns a reference to the derived class. It is part of the
     * Curiously Recurring Template Pattern (CRTP) used to implement polymorphic
     * behavior without the overhead of virtual functions.
     * 
     * @return A reference to the derived class.
     */
    Derived& derived()
    {
        return static_cast<Derived&>(*this);
    }

    beast::flat_buffer buffer_; ///< Buffer used for reading and writing data.

    /**
     * @brief Start reading from the WebSocket stream.
     * 
     * This method initiates an asynchronous read operation on the WebSocket stream.
     * The data read from the stream is stored in the buffer, and the on_read method
     * is called when the read operation completes.
     */
    void do_read()
    {
        derived().ws().async_read(
                buffer_,
                beast::bind_front_handler(
                    &websocket_session::on_read,
                    derived().shared_from_this()));
    }

    /**
     * @brief Handle the result of a read operation.
     * 
     * This method is called when an asynchronous read operation completes. It checks
     * for errors, processes the data read from the stream, and initiates a write
     * operation to send a response back to the client.
     * 
     * @param ec The error code returned by the read operation.
     * @param bytes_transferred The number of bytes read from the stream.
     */
    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // Handle connection closure
        if(ec == websocket::error::closed)
            return;

        if(ec)
            return fail(ec, "read");

        // Echo the message back to the client
        derived().ws().text(derived().ws().got_text());
        derived().ws().async_write(
                buffer_.data(),
                beast::bind_front_handler(
                    &websocket_session::on_write,
                    derived().shared_from_this()));
    }

    /**
     * @brief Handle the result of a write operation.
     * 
     * This method is called when an asynchronous write operation completes. It checks
     * for errors and starts another read operation to continue handling WebSocket
     * communication.
     * 
     * @param ec The error code returned by the write operation.
     * @param bytes_transferred The number of bytes written to the stream.
     */
    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        // Clear the buffer after writing
        buffer_.consume(buffer_.size());

        // Continue reading from the stream
        do_read();
    }

    public:
    /**
     * @brief Start the WebSocket session by accepting an HTTP upgrade request.
     * 
     * This method initiates the WebSocket session by accepting an HTTP upgrade request
     * and setting WebSocket-specific options, such as timeouts and response decorators.
     * 
     * @tparam Body The type of the HTTP request body.
     * @tparam Allocator The type of the allocator used in the HTTP request.
     * @param req The HTTP request that initiated the WebSocket upgrade.
     */
    template<class Body, class Allocator>
        void run(http::request<Body, http::basic_fields<Allocator>> req)
        {
            // Set suggested timeouts for the WebSocket session
            derived().ws().set_option(
                    websocket::stream_base::timeout::suggested(
                        beast::role_type::server));

            // Set a decorator to customize the WebSocket handshake response
            derived().ws().set_option(
                    websocket::stream_base::decorator(
                        [](websocket::response_type& res)
                        {
                        res.set(http::field::server,
                                std::string(BOOST_BEAST_VERSION_STRING) +
                                " advanced-server-flex");
                        }));

            // Accept the WebSocket handshake
            derived().ws().async_accept(
                    req,
                    beast::bind_front_handler(
                        &websocket_session::on_accept,
                        derived().shared_from_this()));
        }

    private:
    /**
     * @brief Handle the result of the WebSocket handshake acceptance.
     * 
     * This method is called when the WebSocket handshake is successfully accepted.
     * It initiates the first read operation to start handling WebSocket communication.
     * 
     * @param ec The error code returned by the accept operation.
     */
    void on_accept(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "accept");

        // Start reading messages from the WebSocket
        do_read();
    }
};

#endif

