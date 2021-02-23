#pragma once

#include <memory>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "common/common.h"
#include "session/handle_request.h"
#include "state/state.h"

#include <boost/optional.hpp>
#include <boost/beast/websocket.hpp>

namespace net
{

    namespace ba = boost::asio;
    namespace ws = beast::websocket;

    // Handles an HTTP server connection
    class Session : public std::enable_shared_from_this<Session>
    {
        // This is the C++11 equivalent of a generic lambda.
        // The function object is used to send an HTTP message.
        struct send_lambda
        {
            Session &self_;

            explicit send_lambda(Session &self)
                : self_(self)
            {
            }

            template <bool isRequest, class Body, class Fields>
            void
            operator()(http::message<isRequest, Body, Fields> &&msg) const
            {
                // The lifetime of the message has to extend
                // for the duration of the async operation so
                // we use a shared_ptr to manage it.
                auto sp = std::make_shared<
                    http::message<isRequest, Body, Fields>>(std::move(msg));

                // Store a type-erased version of the shared
                // pointer in the class to keep it alive.
                // self_.res_ = sp;

                // Write the response
                // http::async_write(
                //     self_.stream_,
                //     *sp,
                //     beast::bind_front_handler(
                //         &Session::on_write,
                //         self_.shared_from_this(),
                //         sp->need_eof()));

                auto self = self_.shared_from_this();
                http::async_write(
                    self_.stream_,
                    *sp,
                    [self, sp](beast::error_code ec, std::size_t bytes) {
                        self->on_write(ec, bytes, sp->need_eof());
                    });
            }
        };

        beast::tcp_stream stream_;
        beast::flat_buffer buffer_;
        std::shared_ptr<State> state_;

        // std::shared_ptr<std::string const> doc_root_;
        // http::request<http::string_body> req_;
        // std::shared_ptr<void> res_;
        send_lambda lambda_;

        // The parser is stored in an optional container so we can
        // construct it from scratch it at the beginning of each new message.
        boost::optional<http::request_parser<http::string_body>> parser_;

    public:
        Session(
            tcp::socket &&,
            std::shared_ptr<State> const &state);

        void run();

    private:
        void do_read();

        void on_read(beast::error_code, std::size_t);

        // void on_write(bool, beast::error_code, std::size_t);
        void on_write(beast::error_code ec, std::size_t, bool close);

        void do_close();
    };

} // end net namespace