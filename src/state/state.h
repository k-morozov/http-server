#pragma once

#include <string>
#include <mutex>
#include <unordered_set>

class ws_session;

class State {
    std::string const doc_root_;
    std::mutex mutex_;

    std::unordered_set<ws_session*> sessions_;

public:
    explicit State(std::string doc_root);

    auto doc_root() const {
        return doc_root_;
    }

    void join(ws_session *);
    void leave(ws_session *);
    void send(std::string);

};