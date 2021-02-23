#pragma once

#include <string>
#include <mutex>
#include <unordered_set>

namespace net
{

class WebSocketSession;

class State {
    std::string const doc_root_;
    std::mutex mutex_;

    std::unordered_set<WebSocketSession*> sessions_;

public:
    explicit State(std::string doc_root);

    auto doc_root() const {
        return doc_root_;
    }

    void join(WebSocketSession *);
    void leave(WebSocketSession *);
    void send(std::string);

};


} // end net namespace