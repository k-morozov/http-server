#include "session/websocket/ws_session.h"
#include <iostream>


namespace net
{

WebSocketSession::WebSocketSession(
    tcp::socket&& socket,
    std::shared_ptr<State> const& state)
    : ws_(std::move(socket))
    , state_(state)
{
}

WebSocketSession::~WebSocketSession()
{
    // Remove this session from the list of active sessions
    state_->leave(this);
}

void WebSocketSession::fail(beast::error_code ec, char const* what)
{
    // Don't report these
    if( ec == boost::asio::error::operation_aborted ||
        ec == ws::error::closed)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void WebSocketSession::on_accept(beast::error_code ec)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "accept");

    // Add this session to the list of active sessions
    state_->join(this);

    // Read a message
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WebSocketSession::on_read,
            shared_from_this()));
}

void WebSocketSession::on_read(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "read");

    // Send to all connections
    state_->send(beast::buffers_to_string(buffer_.data()));

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WebSocketSession::on_read,
            shared_from_this()));
}

void WebSocketSession::send(std::shared_ptr<std::string const> const& ss)
{
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.

    ba::post(
        ws_.get_executor(),
        beast::bind_front_handler(
            &WebSocketSession::on_send,
            shared_from_this(),
            ss));
}

void WebSocketSession::on_send(std::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if(queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(
        ba::buffer(*queue_.front()),
        beast::bind_front_handler(
            &WebSocketSession::on_write,
            shared_from_this()));
}

void WebSocketSession::on_write(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if(! queue_.empty())
        ws_.async_write(
            ba::buffer(*queue_.front()),
            beast::bind_front_handler(
                &WebSocketSession::on_write,
                shared_from_this()));
}


} // end net namespace