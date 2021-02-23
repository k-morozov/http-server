#include "state/state.h"
#include "session/websocket/ws_session.h"
#include <iostream>

namespace net
{

State::State(std::string doc_root)
    : doc_root_(std::move(doc_root))
{

}

void State::join(WebSocketSession* session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(session);
}

void State::leave(WebSocketSession* session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);
}

void State::send(std::string message)
{
    std::cout << "[State::send] " << message << std::endl;

    auto const shared_msg = std::make_shared<std::string const>(std::move(message));

    // Make a local list of all the weak pointers representing
    // the sessions, so we can do the actual sending without
    // holding the mutex:
    std::vector<std::weak_ptr<WebSocketSession>> clients;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        clients.reserve(sessions_.size());
        for(auto session : sessions_)
        {
            clients.emplace_back(session->weak_from_this());
        }
    }

    for(auto const& client : clients)
        if(auto weak_client = client.lock())
        {
            weak_client->send(shared_msg);
        }
}


} // end net namespace