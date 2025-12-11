#include "main.h"
#include "communication/web_interface.h"
#include "communication/http_server.h"
#include "communication/uci.h"

using namespace std;

int main (int argc, char* argv[]) {
    // Mode flags
    bool cliMode = false;    // stdin/stdout JSON driver
    bool serverMode = false; // embedded HTTP API
    bool uciMode = true;     // UCI protocol on stdin/stdout (default)
    if (argc > 1 && string(argv[1]) == "--cli") {
        cliMode = true;
        uciMode = false;
    } else if (argc > 1 && string(argv[1]) == "--server") {
        serverMode = true;
        uciMode = false;
    } else if (argc > 1 && string(argv[1]) == "--uci") {
        uciMode = true;
    } else if (argc > 1 && string(argv[1]) == "--default") {
        uciMode = false;
        serverMode = false;
    }
    
    // Suppress initialization output in cliMode and uciMode
    if (cliMode || uciMode) {
        streambuf* orig_cout = cout.rdbuf();
        streambuf* orig_cerr = cerr.rdbuf();
        stringstream devnull;
        cout.rdbuf(devnull.rdbuf());
        cerr.rdbuf(devnull.rdbuf());
        computeAllTables();
        cout.rdbuf(orig_cout);
        cerr.rdbuf(orig_cerr);
    } else {
        computeAllTables();
    }

    if (uciMode) {
        UCI uci;
        uci.loop();
    } else if (cliMode || serverMode) {
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
        playEngine(STARTING_FEN, 1000);

        // perft(5, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    }

    return 0;
}