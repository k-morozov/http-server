#include "listener/listener.h"

int main(int argc, char* argv[])
{
    // Create and launch a listening port
    try 
    {
        if (argc != 5)
        {
            std::cerr <<
                "Usage: http-server-async <address> <port> <doc_root> <threads>\n" <<
                "Example:\n" <<
                "    http-server-async 0.0.0.0 8080 . 1\n";
            return EXIT_FAILURE;
        }
        auto const address = boost::asio::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
        auto const doc_root = std::string(argv[3]);
        auto const threads = std::max<int>(1, std::atoi(argv[4]));

        boost::asio::io_context ioc{threads};

        auto listener = std::make_shared<net::Listener> (
        ioc,
        net::tcp::endpoint{address, port},
        std::make_shared<State>(doc_root));

        listener->run();

        std::vector<std::thread> v;
        v.reserve(threads - 1);

        for(auto i = threads - 1; i > 0; --i)
            v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
        ioc.run();
    } 
    catch (const std::exception &ex) 
    {
        std::cerr << ex.what() << std::endl;
    }
    

    return EXIT_SUCCESS;
}