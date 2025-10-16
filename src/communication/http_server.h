#pragma once

#include <string>
#include <thread>
#include <atomic>

class WebInterface;

class HttpServer {
public:
    explicit HttpServer(WebInterface& webInterface);
    ~HttpServer();

    bool start(unsigned short port = 8080);
    void stop();

private:
    WebInterface& web;
    std::atomic<bool> running{false};
};


