#!/usr/bin/env python3
"""
Stickshark web bridge.

Spawns the Stickshark engine (UCI, stdin/stdout) and relays the UCI protocol
between the engine and the browser over plain HTTP:

    GET  /events   Server-Sent Events stream of every line the engine prints.
    POST /send     Body is a single UCI command; written to the engine's stdin.
    GET  /...      Static files served from this `web/` directory.

Only the Python standard library is used, so no pip installs are required.

    python3 web/server.py            # serves http://localhost:8000
    python3 web/server.py 9000       # custom port
"""

import os
import sys
import queue
import threading
import subprocess
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

WEB_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(WEB_DIR)
ENGINE_PATH = os.path.join(REPO_ROOT, "build", "stickshark")

# Set of live SSE client queues; each engine output line is fanned out to all.
_clients = set()
_clients_lock = threading.Lock()


def broadcast(line):
    with _clients_lock:
        for q in list(_clients):
            q.put(line)


class Engine:
    """A single persistent UCI engine subprocess."""

    def __init__(self, path):
        self.path = path
        self.proc = None
        self.lock = threading.Lock()
        self.start()

    def start(self):
        self.proc = subprocess.Popen(
            [self.path],
            cwd=REPO_ROOT,           # so the engine finds data/Perfect2023.bin
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            bufsize=1,               # line buffered
        )
        threading.Thread(target=self._pump, daemon=True).start()

    def _pump(self):
        for line in self.proc.stdout:
            broadcast(line.rstrip("\n"))
        broadcast("info string engine process exited")

    def send(self, command):
        with self.lock:
            if self.proc.poll() is not None:
                self.start()            # respawn if it died
            self.proc.stdin.write(command + "\n")
            self.proc.stdin.flush()


class Handler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, *args):
        pass  # keep the console quiet

    # ---- routes -----------------------------------------------------------
    def do_GET(self):
        if self.path == "/events":
            return self.stream_events()
        return self.serve_static()

    def do_POST(self):
        if self.path != "/send":
            self.send_error(404)
            return
        length = int(self.headers.get("Content-Length", 0))
        command = self.rfile.read(length).decode("utf-8").strip()
        if command:
            self.server.engine.send(command)
        self.send_response(204)
        self.send_header("Content-Length", "0")
        self.end_headers()

    # ---- SSE --------------------------------------------------------------
    def stream_events(self):
        q = queue.Queue()
        with _clients_lock:
            _clients.add(q)
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Connection", "keep-alive")
        self.end_headers()
        try:
            while True:
                try:
                    line = q.get(timeout=15)
                    self.wfile.write(f"data: {line}\n\n".encode("utf-8"))
                except queue.Empty:
                    self.wfile.write(b": keepalive\n\n")  # comment ping
                self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError):
            pass
        finally:
            with _clients_lock:
                _clients.discard(q)

    # ---- static files -----------------------------------------------------
    def serve_static(self):
        rel = self.path.split("?", 1)[0].lstrip("/")
        if rel == "":
            rel = "index.html"
        full = os.path.normpath(os.path.join(WEB_DIR, rel))
        if not full.startswith(WEB_DIR) or not os.path.isfile(full):
            self.send_error(404)
            return
        ctype = {
            ".html": "text/html",
            ".js": "text/javascript",
            ".css": "text/css",
            ".svg": "image/svg+xml",
            ".json": "application/json",
            ".ico": "image/x-icon",
        }.get(os.path.splitext(full)[1], "application/octet-stream")
        with open(full, "rb") as f:
            body = f.read()
        self.send_response(200)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    if not os.path.isfile(ENGINE_PATH):
        sys.exit(f"Engine not found at {ENGINE_PATH}\nBuild it first: cmake --build build")

    server = ThreadingHTTPServer(("127.0.0.1", port), Handler)
    server.engine = Engine(ENGINE_PATH)
    print(f"Stickshark web GUI: http://localhost:{port}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.")
        server.engine.proc.terminate()


if __name__ == "__main__":
    main()
