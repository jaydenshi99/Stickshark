#include "main.h"
#include "web/web_interface.h"
#include "web/http_server.h"

using namespace std;

int main (int argc, char* argv[]) {
    // Check for web mode flag
    bool webMode = false;
    bool httpMode = false;
    if (argc > 1 && string(argv[1]) == "--web") {
        webMode = true;
    } else if (argc > 1 && string(argv[1]) == "--http") {
        httpMode = true;
    }
    
    // Suppress initialization output in web/http mode
    if (webMode || httpMode) {
        streambuf* orig = cout.rdbuf();
        stringstream devnull;
        cout.rdbuf(devnull.rdbuf());
        computeAllTables();
        cout.rdbuf(orig);
    } else {
        computeAllTables();
    }

    if (webMode || httpMode) {
        WebInterface webInterface;
        if (httpMode) {
            // Start minimal HTTP server (blocking)
            HttpServer server(webInterface);
            server.start(8080);
        } else {
            string line;
            while (getline(cin, line)) {
                if (line == "quit" || line == "exit") {
                    break;
                }
                string response = webInterface.processCommand(line);
                cout << response << endl;
                cout.flush();
            }
        }
    } else {
        // Original chess game mode
        playEngine(STARTING_FEN, 1000);
    }

    return 0;
}