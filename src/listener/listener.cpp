#include <listener/listener.h>

namespace net {


Listener::Listener(
        ba::io_context& ioc,
        tcp::endpoint endpoint,
        std::shared_ptr<State> state)
        : ioc_(ioc)
        , acceptor_(ba::make_strand(ioc))
        , state_(state)
{
    acceptor_.open(endpoint.protocol());

    acceptor_.set_option(ba::socket_base::reuse_address(true));

    acceptor_.bind(endpoint);

    acceptor_.listen(
        ba::socket_base::max_listen_connections);
}


void Listener::do_accept()
{
    acceptor_.async_accept(
        ba::make_strand(ioc_),
        beast::bind_front_handler(
            &Listener::on_accept,
            shared_from_this()));
}


void Listener::on_accept(beast::error_code ec, tcp::socket socket)
{
    if(!ec)
    {
        std::make_shared<HttpSession>(std::move(socket), state_)->run();
    }
    else
    {
        fail(ec, "accept");
    }

    do_accept();
}


} // end net namespace