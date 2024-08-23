#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// Include the new headers
#include "../include/util/server_certificate.hpp"
#include "../include/http/listener.hpp"
#include "../include/http/detect_session.hpp"
#include "../include/http/plain_http_session.hpp"
#include "../include/http/ssl_http_session.hpp"
#include "../include/http/request_handler.hpp"
#include "../include/websocket/plain_websocket_session.hpp"
#include "../include/websocket/ssl_websocket_session.hpp"
#include "../include/websocket/websocket_session.hpp"
#include "../include/websocket/websocket_factory.hpp"

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cerr <<
            "Usage: advanced-server-flex <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    advanced-server-flex 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::string>(argv[3]);
    auto const threads = std::max<int>(1, std::atoi(argv[4]));

    net::io_context ioc{threads};

    ssl::context ctx{ssl::context::tlsv12};

    load_server_certificate(ctx);

    std::make_shared<listener>(
        ioc,
        ctx,
        tcp::endpoint{address, port},
        doc_root)->run();

    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&](beast::error_code const&, int)
        {
            ioc.stop();
        });

    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    for(auto& t : v)
        t.join();

    return EXIT_SUCCESS;
}

