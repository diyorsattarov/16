#ifndef LISTENER_HPP
#define LISTENER_HPP

#include "../util/beast.hpp"
#include "../util/util.hpp"
#include "detect_session.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>

/**
 * @brief The listener class is responsible for accepting incoming TCP connections.
 * 
 * This class is designed to accept incoming TCP connections on a specified endpoint,
 * create a new session for each connection, and handle the incoming requests asynchronously.
 */
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_; ///< The I/O context to be used for asynchronous operations.
    ssl::context& ctx_; ///< The SSL context used for managing SSL settings and certificates.
    tcp::acceptor acceptor_; ///< The acceptor object that will listen for new connections.
    std::shared_ptr<std::string const> doc_root_; ///< The root directory for serving HTTP content.

    public:
    /**
     * @brief Constructor for the listener class.
     * 
     * This constructor initializes the listener with the I/O context, SSL context, endpoint, and document root.
     * It also sets up the acceptor to listen for incoming connections.
     * 
     * @param ioc The I/O context to use for asynchronous operations.
     * @param ctx The SSL context used to manage SSL settings and certificates.
     * @param endpoint The TCP endpoint on which to listen for incoming connections.
     * @param doc_root The root directory for serving HTTP content.
     */
    listener(net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root)
        : ioc_(ioc)
          , ctx_(ctx)
          , acceptor_(net::make_strand(ioc))
          , doc_root_(doc_root)
    {
        beast::error_code ec;

        // Open the acceptor using the specified endpoint's protocol.
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow the address to be reused to avoid conflicts on restart.
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind the acceptor to the endpoint.
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for incoming connections.
        acceptor_.listen(
                net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    /**
     * @brief Start accepting incoming connections.
     * 
     * This method initiates the asynchronous accept operation, allowing the server to start
     * handling incoming connections.
     */
    void run()
    {
        do_accept();
    }

    private:
    /**
     * @brief Asynchronously accept a new incoming connection.
     * 
     * This method is responsible for accepting new connections and creating a session to handle
     * each connection. It recursively calls itself to continuously accept new connections.
     */
    void do_accept()
    {
        acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                    &listener::on_accept,
                    shared_from_this()));
    }

    /**
     * @brief Handle the accepted connection.
     * 
     * This method is called when a new connection is accepted. It creates a session
     * (either plain or SSL) to handle the connection and starts processing the request.
     * 
     * @param ec The error code from the accept operation.
     * @param socket The socket representing the new connection.
     */
    void on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create a new session to handle the connection.
            std::make_shared<detect_session>(
                    std::move(socket),
                    ctx_,
                    doc_root_)->run();
        }

        // Accept the next connection.
        do_accept();
    }
};

#endif // LISTENER_HPP

