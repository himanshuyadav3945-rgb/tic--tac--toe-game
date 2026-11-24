#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketType = SOCKET;
    #define CLOSE_SOCKET closesocket
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    using SocketType = int;
    #define INVALID_SOCKET -1
    #define CLOSE_SOCKET close
#endif

using namespace std;

char board[9];
char currentPlayer = 'X';
bool gameActive = true;
string gameStatus = "Player X's turn";

const int WINNING_CONDITIONS[8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // Rows
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // Columns
    {0, 4, 8}, {2, 4, 6}             // Diagonals
};

void resetGame() {
    for (int i = 0; i < 9; i++) {
        board[i] = ' ';
    }
    currentPlayer = 'X';
    gameActive = true;
    gameStatus = "Player X's turn";
}

bool checkWin() {
    for (int i = 0; i < 8; i++) {
        int a = WINNING_CONDITIONS[i][0];
        int b = WINNING_CONDITIONS[i][1];
        int c = WINNING_CONDITIONS[i][2];

        if (board[a] != ' ' && board[a] == board[b] && board[b] == board[c]) {
            gameActive = false;
            gameStatus = "Player " + string(1, currentPlayer) + " wins!";
            return true;
        }
    }
    return false;
}

bool checkDraw() {
    for (int i = 0; i < 9; i++) {
        if (board[i] == ' ') {
            return false;
        }
    }
    gameActive = false;
    gameStatus = "It's a draw!";
    return true;
}

string buildJSONResponse() {
    stringstream json;
    json << "{";
    json << "\"board\": [";
    for (int i = 0; i < 9; i++) {
        json << "\"" << board[i] << "\"";
        if (i < 8) json << ",";
    }
    json << "],";
    json << "\"currentPlayer\": \"" << currentPlayer << "\",";
    json << "\"gameactive\": " << (gameActive ? "true" : "false") << ",";
    json << "\"gamestatus\": \"" << gameStatus << "\"";
    json << "}";

    string body = json.str();

    stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Content-Type: application/json\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;

    return response.str();
}

int main() {
    resetGame();

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }
#endif

    SocketType server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        return 1;
    }

    // Allow immediate port reuse after restarting the server
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8081);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed. Port 8080 may already be in use." << endl;
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        cerr << "Listen failed." << endl;
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    cout << "C++ Tic-Tac-Toe server started at http://localhost:8080" << endl;

    while (true) {
        SocketType new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket == INVALID_SOCKET) {
            continue;
        }

        char buffer[2048] = {0};
        int bytesReceived = recv(new_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            CLOSE_SOCKET(new_socket);
            continue;
        }

        string request(buffer);
        string response;

        if (request.find("OPTIONS ") == 0) {
            response = "HTTP/1.1 204 No Content\r\n"
                       "Access-Control-Allow-Origin: *\r\n"
                       "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                       "Access-Control-Allow-Headers: Content-Type\r\n"
                       "Connection: close\r\n\r\n";
        } 
        else if (request.find("GET /state") == 0) {
            // Fixed: Removed resetGame() here so fetching state doesn't wipe the board
            response = buildJSONResponse();
        } 
        else if (request.find("POST /reset") == 0) {
            resetGame();
            response = buildJSONResponse();
        } 
        else if (request.find("POST /move") == 0) {
            size_t pos = request.find("index=");
            if (pos != string::npos && gameActive && (pos + 6 < request.length())) {
                int index = request[pos + 6] - '0';
                if (index >= 0 && index < 9 && board[index] == ' ') {
                    board[index] = currentPlayer;
                    if (!checkWin() && !checkDraw()) {
                        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
                        gameStatus = string("Player ") + currentPlayer + "'s turn";
                    }
                }
            }
            // Fixed: Always send a response, even if the game is over or index is invalid
            response = buildJSONResponse();
        } 
        else {
            response = "HTTP/1.1 404 Not Found\r\n"
                       "Access-Control-Allow-Origin: *\r\n"
                       "Content-Length: 9\r\n"
                       "Connection: close\r\n\r\n"
                       "Not Found";
        }

        send(new_socket, response.c_str(), static_cast<int>(response.length()), 0);
        CLOSE_SOCKET(new_socket);
    }

    CLOSE_SOCKET(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}