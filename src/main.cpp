#include "main.h"
#include "web_interface.h"

using namespace std;

int main (int argc, char* argv[]) {
    // Check for web mode flag
    bool webMode = false;
    if (argc > 1 && string(argv[1]) == "--web") {
        webMode = true;
    }
    
    // Suppress initialization output in web mode
    if (webMode) {
        streambuf* orig = cout.rdbuf();
        stringstream devnull;
        cout.rdbuf(devnull.rdbuf());
        computeAllTables();
        cout.rdbuf(orig);
    } else {
        computeAllTables();
    }

    if (webMode) {
        // Web interface mode - read commands from stdin
        WebInterface webInterface;
        string line;
        
        while (getline(cin, line)) {
            if (line == "quit" || line == "exit") {
                break;
            }
            
            string response = webInterface.processCommand(line);
            cout << response << endl;
            cout.flush();
        }
    } else {
        // Original chess game mode
        playEngine(STARTING_FEN, 1000);
    }

    return 0;
}