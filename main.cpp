#include <iostream>
#include <memory>
#include <thread>
#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace ba = boost::asio;
namespace bb = boost::beast;

using tcp = boost::asio::ip::tcp;

void do_session(tcp::socket&, std::shared_ptr<std::string>) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}


int main(int argc, char * argv[]) {
    try {
        if (argc != 4) {
            std::cerr << "count params" << "\n";
            return EXIT_FAILURE;
        }
        
        auto const address = ba::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::stoi(argv[2]));
        auto const doc_root = std::make_shared<std::string>(argv[3]);

        ba::io_context  ioc_{1};

        tcp::acceptor acc_{ioc_, {address, port}};

        for(;;) {
            tcp::socket sock(ioc_);
            acc_.accept(sock);

            std::thread th(std::bind(&do_session, std::move(sock), doc_root));
            th.detach();
        }

    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    
}