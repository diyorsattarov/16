#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <string>
#include <memory>

namespace beast = boost::beast; // Namespace alias for Boost.Beast
namespace http = beast::http;   // Namespace alias for Boost.Beast's HTTP module

/**
 * @brief Determine the MIME type based on the file extension.
 * 
 * This function examines the file extension of the given path and returns the 
 * corresponding MIME type as a string view. This is useful for setting the 
 * Content-Type header in HTTP responses.
 * 
 * @param path The file path from which to determine the MIME type.
 * @return A string view representing the MIME type.
 */
beast::string_view mime_type(beast::string_view path)
{
    using beast::iequals; // Case-insensitive comparison

    // Lambda function to extract the file extension from the path
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{}; // Return an empty string view if no extension is found
        return path.substr(pos); // Return the extension (e.g., ".html")
    }();

    // Compare the file extension with known types and return the corresponding MIME type
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
    return "application/text"; // Default MIME type
}

/**
 * @brief Concatenate a base path and a relative path.
 * 
 * This function concatenates a base directory path with a relative file path,
 * ensuring that the resulting path is valid for the operating system. It handles
 * path separators appropriately for both Windows and POSIX systems.
 * 
 * @param base The base directory path.
 * @param path The relative file path.
 * @return A concatenated string representing the full file path.
 */
std::string path_cat(beast::string_view base, beast::string_view path)
{
    if(base.empty())
        return std::string(path);

    std::string result(base);

#ifdef BOOST_MSVC // Windows-specific path handling
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else // POSIX-specific path handling
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif

    return result;
}

/**
 * @brief Handle an HTTP request and produce an appropriate response.
 * 
 * This function processes an incoming HTTP request, determines the appropriate
 * response (e.g., serving a file or returning an error), and generates the
 * HTTP response message. It handles different HTTP methods, checks for the
 * existence of requested resources, and sets appropriate headers.
 * 
 * @tparam Body The type of the request body.
 * @tparam Allocator The type of the allocator used by the request.
 * @param doc_root The root directory from which to serve files.
 * @param req The HTTP request to handle.
 * @return A generator for producing the HTTP response message.
 */
template<class Body, class Allocator>
http::message_generator handle_request(beast::string_view doc_root, http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // Lambda function to generate a "400 Bad Request" response
    auto const bad_request = [&req](beast::string_view why)
    {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Lambda function to generate a "404 Not Found" response
    auto const not_found = [&req](beast::string_view target)
    {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Lambda function to generate a "500 Internal Server Error" response
    auto const server_error = [&req](beast::string_view what)
    {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    // Validate the request method (only GET and HEAD are supported)
    if( req.method() != http::verb::get && req.method() != http::verb::head)
        return bad_request("Unknown HTTP-method");

    // Ensure the request target is valid and does not attempt directory traversal
    if( req.target().empty() || req.target()[0] != '/' || req.target().find("..") != beast::string_view::npos)
        return bad_request("Illegal request-target");

    // Concatenate the document root with the request target to form the full file path
    std::string path = path_cat(doc_root, req.target());

    // If the request target ends with a '/', append "index.html"
    if(req.target().back() == '/')
        path.append("index.html");

    beast::error_code ec;
    http::file_body::value_type body;

    // Attempt to open the file at the computed path
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // If the file was not found, return a "404 Not Found" response
    if(ec == beast::errc::no_such_file_or_directory)
        return not_found(req.target());

    // If there was another error opening the file, return a "500 Internal Server Error" response
    if(ec)
        return server_error(ec.message());

    auto const size = body.size(); // Get the file size

    // Handle HEAD requests by returning only the headers
    if(req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    // Handle GET requests by returning the file content
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

#endif // REQUEST_HANDLER_HPP

