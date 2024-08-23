#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <string>
#include <memory>

namespace beast = boost::beast; // Namespace alias for Boost.Beast
namespace http = beast::http;   // Namespace alias for Boost.Beast's HTTP module

// Determine the MIME type based on the file extension
inline beast::string_view mime_type(beast::string_view path)
{
    using beast::iequals; // Case-insensitive comparison

    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();

    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Concatenate a base path and a relative path
inline std::string path_cat(beast::string_view base, beast::string_view path)
{
    if(base.empty())
        return std::string(path);

    std::string result(base);

#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif

    return result;
}

// Send an HTTP response with the given status and body
template<class Body, class Allocator>
http::message_generator send_(
    http::request<Body, http::basic_fields<Allocator>> const& req,
    http::status status,
    beast::string_view body,
    beast::string_view content_type = "text/html")
{
    http::response<http::string_body> res{status, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, content_type);
    res.keep_alive(req.keep_alive());
    res.body() = std::string(body);
    res.prepare_payload();
    return res;
}

// Handle GET and HEAD requests
template<class Body, class Allocator>
http::message_generator handle_get(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req)
{
    std::string path = path_cat(doc_root, req.target());
    if(req.target().back() == '/')
        path.append("index.html");

    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    if(ec == beast::errc::no_such_file_or_directory)
        return send_(req, http::status::not_found, "The resource was not found.");

    if(ec)
        return send_(req, http::status::internal_server_error, ec.message());

    auto const size = body.size();

    if(req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return res;
}

// Handle POST requests
template<class Body, class Allocator>
http::message_generator handle_post(
    http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // Handle the POST request. You may need to parse JSON or handle form data here.
    // For demonstration, we'll just return a simple message.
    return send_(req, http::status::ok, "POST request received.");
}

// Handle PUT requests
template<class Body, class Allocator>
http::message_generator handle_put(
    http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // Handle the PUT request. You may need to update resources on the server.
    // For demonstration, we'll just return a simple message.
    return send_(req, http::status::ok, "PUT request received.");
}

// Handle DELETE requests
template<class Body, class Allocator>
http::message_generator handle_delete(
    http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // Handle the DELETE request. You may need to delete resources on the server.
    // For demonstration, we'll just return a simple message.
    return send_(req, http::status::ok, "DELETE request received.");
}

// Main request handler that delegates to specific methods
template<class Body, class Allocator>
http::message_generator handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req)
{
    switch(req.method())
    {
        case http::verb::get:
        case http::verb::head:
            return handle_get(doc_root, std::move(req));
        case http::verb::post:
            return handle_post(std::move(req));
        case http::verb::put:
            return handle_put(std::move(req));
        case http::verb::delete_:
            return handle_delete(std::move(req));
        default:
            return send_(req, http::status::bad_request, "Unknown HTTP-method");
    }
}

#endif // REQUEST_HANDLER_HPP

