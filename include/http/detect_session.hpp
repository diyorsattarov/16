#ifndef DETECT_SESSION_HPP
#define DETECT_SESSION_HPP

#include "http_session.hpp"
#include "ssl_http_session.hpp"
#include "plain_http_session.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

class detect_session : public std::enable_shared_from_this<detect_session>
{
    beast::tcp_stream stream_;
    ssl::context& ctx_;
    std::shared_ptr<std::string const> doc_root_;
    beast::flat_buffer buffer_;

public:
    detect_session(tcp::socket&& socket, ssl::context& ctx, std::shared_ptr<std::string const> const& doc_root)
        : stream_(std::move(socket))
        , ctx_(ctx)
        , doc_root_(doc_root)
    {
    }

    void run()
    {
        net::dispatch(
            stream_.get_executor(),
            beast::bind_front_handler(
                &detect_session::on_run,
                this->shared_from_this()));
    }

    void on_run()
    {
        stream_.expires_after(std::chrono::seconds(30));

        beast::async_detect_ssl(
            stream_,
            buffer_,
            beast::bind_front_handler(
                &detect_session::on_detect,
                this->shared_from_this()));
    }

    void on_detect(beast::error_code ec, bool result)
    {
        if(ec)
            return fail(ec, "detect");

        if(result)
        {
            // Launch SSL session with doc_root
            std::make_shared<ssl_http_session>(
                    std::move(stream_),
                    ctx_,
                    std::move(buffer_),
                    doc_root_)->run(); // Pass doc_root_ here
            return;
        }

        // Launch plain session with doc_root
        std::make_shared<plain_http_session>(
                std::move(stream_),
                std::move(buffer_),
                doc_root_)->run(); // Pass doc_root_ here
    }

};

#endif
