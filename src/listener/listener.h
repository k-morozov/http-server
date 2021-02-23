#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "session/session.h"

namespace net {


namespace ba = boost::asio;
namespace beast = boost::beast;

using tcp = boost::asio::ip::tcp;

class Listener : public std::enable_shared_from_this<Listener>
{
    ba::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;

public:
    Listener(ba::io_context&, tcp::endpoint, std::shared_ptr<std::string const> const&);

    void run()
    {
        do_accept();
    }

private:
    void do_accept();

    void on_accept(beast::error_code ec, tcp::socket socket);
};


} // end net namespace