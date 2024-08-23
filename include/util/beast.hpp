//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#ifndef BEAST_HPP
#define BEAST_HPP

#include <boost/beast.hpp>
#include <boost/config.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

// Namespace aliases for Boost.Beast and Boost.Asio components.
namespace beast = boost::beast;                 // Namespace for Boost.Beast, provides core I/O and HTTP functionality.
namespace http = beast::http;                   // Namespace for HTTP-related classes and functions in Boost.Beast.
namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>
namespace net = boost::asio;                    // Namespace for Boost.Asio, provides I/O functionality.
namespace ssl = boost::asio::ssl;               // Namespace for SSL-related classes and functions in Boost.Asio.
using tcp = boost::asio::ip::tcp;               // Alias for TCP socket type in Boost.Asio.

#endif // BEAST_HPP

