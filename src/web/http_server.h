#pragma once

#include <string>
#include <thread>
#include <atomic>

class WebInterface;

class HttpServer {
public:
    explicit HttpServer(WebInterface& webInterface);
    ~HttpServer();

    // Start blocking HTTP server on given port (e.g., 8080). Returns when stop() is called or on error.
    bool start(unsigned short port = 8080);
    void stop();

private:
    WebInterface& web;
    std::atomic<bool> running{false};
};


