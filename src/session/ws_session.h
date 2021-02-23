#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "state/state.h"
#include <memory>

// namespace net = boost::asio;
namespace beast = boost::beast;                 
namespace http = beast::http;
namespace ws = beast::websocket; 

using tcp = boost::asio::ip::tcp; 


class State;

/** Represents an active WebSocket connection to the server
*/
class ws_session : public std::enable_shared_from_this<ws_session>
{
    beast::flat_buffer buffer_;
    ws::stream<beast::tcp_stream> ws_;
    std::shared_ptr<State> state_;
    std::vector<std::shared_ptr<std::string const>> queue_;

    void fail(beast::error_code ec, char const* what);
    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

public:
    ws_session(
        tcp::socket&& socket,
        std::shared_ptr<State> const& state);

    ~ws_session();

    template<class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req);

    // Send a message
    void send(std::shared_ptr<std::string const> const& ss);

private:
    void
    on_send(std::shared_ptr<std::string const> const& ss);
};

template<class Body, class Allocator>
void ws_session::run(http::request<Body, http::basic_fields<Allocator>> req)
{
    // Set suggested timeout settings for the websocket
    ws_.set_option(
        ws::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(ws::stream_base::decorator(
        [](ws::response_type& res)
        {
            res.set(http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-chat-multi");
        }));

    // Accept the websocket handshake
    ws_.async_accept(
        req,
        beast::bind_front_handler(
            &ws_session::on_accept,
            shared_from_this()));
}