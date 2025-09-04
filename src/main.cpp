#include "main.h"
#include "web/web_interface.h"
#include "web/http_server.h"

using namespace std;

int main (int argc, char* argv[]) {
    // Mode flags
    bool cliMode = false;    // stdin/stdout JSON driver
    bool serverMode = false; // embedded HTTP API
    if (argc > 1 && string(argv[1]) == "--cli") {
        cliMode = true;
    } else if (argc > 1 && string(argv[1]) == "--server") {
        serverMode = true;
    }
    
    // Suppress initialization output in cliMode
    if (cliMode) {
        streambuf* orig = cout.rdbuf();
        stringstream devnull;
        cout.rdbuf(devnull.rdbuf());
        computeAllTables();
        cout.rdbuf(orig);
    } else {
        computeAllTables();
    }

    if (cliMode || serverMode) {
        WebInterface webInterface;
        webInterface.setQuiet(cliMode);
        if (serverMode) {
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
        // playEngine(STARTING_FEN, 1000);

        Board board;
        board.setStartingPosition();

        Engine engine(board);
        engine.findBestMove(5000);
    }

    return 0;
}