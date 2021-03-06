#include <session/http/http_session.h>
#include <session/websocket/ws_session.h>


namespace net {


HttpSession::HttpSession(
    tcp::socket &&socket,
    std::shared_ptr<State> const& state)
    : stream_(std::move(socket)),
    state_(state), lambda_(*this)
{
}

void HttpSession::run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    // ba::dispatch(stream_.get_executor(),
    //              beast::bind_front_handler(
    //                  &Session::do_read,
    //                  shared_from_this()));
    do_read();
}

void HttpSession::do_read()
{
    // Construct a new parser for each message
    parser_.emplace();
    
    // Apply a reasonable limit to the allowed size
    // of the body in bytes to prevent abuse.
    parser_->body_limit(10000);

    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(
        stream_,
        buffer_,
        parser_->get(),
        beast::bind_front_handler(
            &HttpSession::on_read,
            shared_from_this()));
}

void HttpSession::on_read(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream)
        return do_close();

    if (ec)
        return fail(ec, "read");

    
    // See if it is a WebSocket Upgrade
    if(ws::is_upgrade(parser_->get()))
    {
        std::cout << "request: ws" << '\n';
        // Create a websocket session, transferring ownership
        // of both the socket and the HTTP request.
        auto ws_session_ = std::make_shared<WebSocketSession>(
            stream_.release_socket(),
                state_);
        ws_session_->run(parser_->release());
    } else {
        std::cout << "request: http" << '\n';
        handle_request(state_->doc_root(), parser_->release(), lambda_);
    }
    
}

void HttpSession::do_close()
{
    // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

void HttpSession::on_write(beast::error_code ec, std::size_t, bool close)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

    if(close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    // Read another request
    do_read();
}


} // end net namespace