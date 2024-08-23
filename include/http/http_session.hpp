#ifndef HTTP_SESSION_HPP
#define HTTP_SESSION_HPP

#include "../util/util.hpp"
#include "request_handler.hpp"
#include "../websocket/websocket_factory.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <queue>

/**
 * @brief The http_session class template provides the functionality to manage an HTTP session.
 * 
 * It handles HTTP requests, manages the reading and writing of data over a network connection,
 * and can also upgrade the connection to a WebSocket session if requested by the client.
 * 
 * This class is designed to be used with the Curiously Recurring Template Pattern (CRTP),
 * where the derived class specifies the concrete type.
 * 
 * @tparam Derived The derived class that specifies the concrete implementation.
 */
template<class Derived>
class http_session
{
    std::shared_ptr<std::string const> doc_root_;  ///< The root directory for serving HTTP content.

    /**
     * @brief Access the derived class, following the CRTP pattern.
     * 
     * This method allows access to the derived class from the base class template.
     * 
     * @return A reference to the derived class.
     */
    Derived& derived()
    {
        return static_cast<Derived&>(*this);
    }

    static constexpr std::size_t queue_limit = 8; ///< Maximum number of responses in the queue.
    std::queue<http::message_generator> response_queue_; ///< Queue to manage outgoing responses.

    /**
     * @brief Parser for the incoming HTTP request.
     * 
     * The parser is stored in an optional container so that it can be constructed
     * anew for each incoming message, ensuring a clean state for each request.
     */
    boost::optional<http::request_parser<http::string_body>> parser_;

    protected:
    beast::flat_buffer buffer_; ///< Buffer for reading data from the stream.

    public:
    /**
     * @brief Constructor for the http_session class.
     * 
     * @param buffer The buffer to use for reading data from the stream.
     * @param doc_root The root directory for serving HTTP content.
     */
    http_session(
            beast::flat_buffer buffer,
            std::shared_ptr<std::string const> const& doc_root)
        : doc_root_(doc_root)
          , buffer_(std::move(buffer))
    {
    }

    /**
     * @brief Start reading from the stream.
     * 
     * This method initiates the asynchronous reading of an HTTP request from the stream.
     */
    void do_read()
    {
        // Construct a new parser for each incoming message.
        parser_.emplace();

        // Apply a reasonable limit to the allowed size of the body in bytes to prevent abuse.
        parser_->body_limit(10000);

        // Set the timeout for reading the request.
        beast::get_lowest_layer(
                derived().stream()).expires_after(std::chrono::seconds(30));

        // Start reading the request asynchronously using the parser-oriented interface.
        http::async_read(
                derived().stream(),
                buffer_,
                *parser_,
                beast::bind_front_handler(
                    &http_session::on_read,
                    derived().shared_from_this()));
    }

    /**
     * @brief Handle the completion of the read operation.
     * 
     * This method is called when the asynchronous read operation completes. It processes the
     * HTTP request, checks for a WebSocket upgrade, and queues the response.
     * 
     * @param ec The error code from the read operation.
     * @param bytes_transferred The number of bytes transferred during the read operation.
     */
    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // If the connection was closed by the client, close the session.
        if(ec == http::error::end_of_stream)
            return derived().do_eof();

        if(ec)
            return fail(ec, "read");

        // Check if the request is asking to upgrade to a WebSocket connection.
        if(websocket::is_upgrade(parser_->get()))
        {
            // Disable the timeout, as WebSocket has its own timeout management.
            beast::get_lowest_layer(derived().stream()).expires_never();

            // Create a WebSocket session and transfer ownership of the socket and request.
            return make_websocket_session(
                    derived().release_stream(),
                    parser_->release());
        }

        // Handle the HTTP request and queue the response.
        queue_write(handle_request(*doc_root_, parser_->release()));

        // If the response queue is not full, read the next request.
        if (response_queue_.size() < queue_limit)
            do_read();
    }

    /**
     * @brief Queue a response for writing.
     * 
     * This method adds a response to the queue and starts the write loop if it's not already running.
     * 
     * @param response The HTTP response to be queued for writing.
     */
    void queue_write(http::message_generator response)
    {
        // Add the response to the queue.
        response_queue_.push(std::move(response));

        // If this is the only response in the queue, start the write loop.
        if (response_queue_.size() == 1)
            do_write();
    }

    /**
     * @brief Start or continue the write loop.
     * 
     * This method is called to write responses from the queue to the stream.
     * It should not be called if the write loop is already active.
     */
    void do_write()
    {
        if(! response_queue_.empty())
        {
            bool keep_alive = response_queue_.front().keep_alive();

            // Write the response asynchronously.
            beast::async_write(
                    derived().stream(),
                    std::move(response_queue_.front()),
                    beast::bind_front_handler(
                        &http_session::on_write,
                        derived().shared_from_this(),
                        keep_alive));
        }
    }

    /**
     * @brief Handle the completion of the write operation.
     * 
     * This method is called when the asynchronous write operation completes. It checks
     * if the connection should be kept alive and either resumes reading or closes the session.
     * 
     * @param keep_alive Whether to keep the connection alive.
     * @param ec The error code from the write operation.
     * @param bytes_transferred The number of bytes transferred during the write operation.
     */
    void on_write(
            bool keep_alive,
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(! keep_alive)
        {
            // Close the connection if the response indicated "Connection: close".
            return derived().do_eof();
        }

        // If the response queue is full, resume reading for the next request.
        if(response_queue_.size() == queue_limit)
            do_read();

        // Remove the response that was just written.
        response_queue_.pop();

        // Continue writing the next response in the queue.
        do_write();
    }
};

#endif // HTTP_SESSION_HPP

