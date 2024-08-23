#ifndef WEBSOCKET_SESSION_HPP
#define WEBSOCKET_SESSION_HPP

#include "../util/util.hpp"
#include "../util/beast.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <queue>

template<class Derived>
class websocket_session
{
    Derived& derived()
    {
        return static_cast<Derived&>(*this);
    }

    beast::flat_buffer buffer_;

    void do_read()
    {
        derived().ws().async_read(
            buffer_,
            beast::bind_front_handler(
                &websocket_session::on_read,
                derived().shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec == websocket::error::closed)
            return;

        if(ec)
            return fail(ec, "read");

        derived().ws().text(derived().ws().got_text());
        derived().ws().async_write(
            buffer_.data(),
            beast::bind_front_handler(
                &websocket_session::on_write,
                derived().shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        buffer_.consume(buffer_.size());
        do_read();
    }

public:
    template<class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req)
    {
        derived().ws().set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::server));

        derived().ws().set_option(
            websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " advanced-server-flex");
            }));

        derived().ws().async_accept(
            req,
            beast::bind_front_handler(
                &websocket_session::on_accept,
                derived().shared_from_this()));
    }

private:
    void on_accept(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "accept");

        do_read();
    }
};

#endif
