#include "http_server.h"
#include "web_interface.h"

#include <string>
#include <cstring>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using std::string;

namespace {
    // Very small helper to write entire buffer
    bool write_all(int fd, const char* buf, size_t len) {
        size_t written = 0;
        while (written < len) {
            ssize_t n = ::send(fd, buf + written, len - written, 0);
            if (n <= 0) return false;
            written += static_cast<size_t>(n);
        }
        return true;
    }
}

HttpServer::HttpServer(WebInterface& webInterface) : web(webInterface) {}

HttpServer::~HttpServer() { stop(); }

bool HttpServer::start(unsigned short port) {
    if (running.exchange(true)) return false;

    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        running = false;
        return false;
    }

    int opt = 1;
    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (::bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(server_fd);
        running = false;
        return false;
    }

    if (::listen(server_fd, 8) < 0) {
        ::close(server_fd);
        running = false;
        return false;
    }

    while (running.load()) {
        int client_fd = ::accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            if (!running.load()) break;
            continue;
        }

        // Read request (very small parser: only method + path)
        char buf[4096];
        ssize_t n = ::recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { ::close(client_fd); continue; }
        buf[n] = '\0';

        string req(buf);
        // Find method and path
        size_t sp1 = req.find(' ');
        size_t sp2 = sp1 == string::npos ? string::npos : req.find(' ', sp1 + 1);
        string method = sp1 == string::npos ? "" : req.substr(0, sp1);
        string path = sp2 == string::npos ? "/" : req.substr(sp1 + 1, sp2 - sp1 - 1);

        // Find body if any
        size_t header_end = req.find("\r\n\r\n");
        string body = header_end == string::npos ? string("") : req.substr(header_end + 4);

        string json;
        int status = 200;

        if (method == "GET" && path == "/board") {
            json = web.handleGetBoard();
        } else if (method == "GET" && path == "/legal") {
            json = web.handleGetLegal();
        } else if (method == "POST" && path == "/newgame") {
            json = web.handleNewGame();
        } else if (method == "POST" && path == "/enginemove") {
            // Optional: parse timeMs from body later; default
            json = web.handleEngineMove(1000);
        } else if (method == "POST" && path == "/move") {
            // Accept either {id} or {from,to,flag} or legacy {move:"e2-e4"}
            if (body.find("\"id\"") != string::npos ||
                body.find("\"from\"") != string::npos ||
                body.find("\"move\"") != string::npos) {
                json = web.handleValidatedMove(body);
            } else {
                status = 400; json = web.errorResponse("Invalid move payload");
            }
        } else if (method == "OPTIONS") {
            // Handle CORS preflight requests
            status = 200;
            json = ""; // No body needed for OPTIONS
        } else {
            status = 404;
            json = web.errorResponse("Not found");
        }

        // CORS and JSON headers
        string resp = "HTTP/1.1 " + string(status == 200 ? "200 OK" : (status == 400 ? "400 Bad Request" : "404 Not Found")) + "\r\n";
        resp += "Content-Type: application/json\r\n";
        resp += "Access-Control-Allow-Origin: *\r\n";
        resp += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        resp += "Access-Control-Allow-Headers: Content-Type\r\n";
        resp += "Content-Length: " + std::to_string(json.size()) + "\r\n";
        resp += "Connection: close\r\n\r\n";
        resp += json;

        write_all(client_fd, resp.c_str(), resp.size());
        ::close(client_fd);
    }

    ::close(server_fd);
    running = false;
    return true;
}

void HttpServer::stop() {
    running = false;
}


