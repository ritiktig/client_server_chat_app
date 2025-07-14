#include <iostream>         // For standard input/output
#include <vector>           // To use std::vector for storing clients
#include <winsock2.h>       // Winsock2 API for networking
#include <ws2tcpip.h>       // Advanced Winsock functions (not used much here)
#include <windows.h>        // For Windows-specific APIs like threads and critical sections

#pragma comment(lib, "ws2_32.lib")  // Link with Winsock library

#define PORT 9001            // Define server port

std::vector<SOCKET> clients;       // List to store connected client sockets
CRITICAL_SECTION cs;               // Critical section for synchronizing access to clients

// Function to handle each client in a new thread
DWORD WINAPI ClientHandler(LPVOID clientSocket) {
    SOCKET sock = *(SOCKET*)clientSocket;   // Extract the socket from the void pointer
    char buffer[1024];                      // Buffer to store received messages

    while (true) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0); // Receive data from client
        if (bytesReceived <= 0) {          // Check if client disconnected or error
            EnterCriticalSection(&cs);     // Lock access to shared clients vector
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                if (*it == sock) {         // Find the disconnected socket
                    clients.erase(it);     // Remove it from the clients list
                    break;
                }
            }
            LeaveCriticalSection(&cs);     // Unlock the critical section
            closesocket(sock);             // Close the client socket
            break;                         // Exit the thread
        }

        buffer[bytesReceived] = '\0';      // Null-terminate the received string

        // Broadcast the received message to all other clients
        EnterCriticalSection(&cs);         // Lock access to clients list
        for (SOCKET client : clients) {
            if (client != sock) {          // Don't send back to sender
                send(client, buffer, bytesReceived, 0); // Send message to client
            }
        }
        LeaveCriticalSection(&cs);         // Unlock after broadcasting
    }
    return 0;                              // Thread ends
}

int main() {
    WSADATA wsaData;                                      // Structure to hold Winsock info
    WSAStartup(MAKEWORD(2,2), &wsaData);                  // Initialize Winsock with version 2.2
    InitializeCriticalSection(&cs);                       // Initialize critical section

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket using IPv4

    sockaddr_in serverAddress;                            // Structure for server address
    serverAddress.sin_family = AF_INET;                   // Use IPv4
    serverAddress.sin_addr.s_addr = INADDR_ANY;           // Accept connections from any IP
    serverAddress.sin_port = htons(PORT);                 // Convert port to network byte order

    bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)); // Bind socket
    listen(serverSocket, 5);                              // Start listening with a backlog of 5

    std::cout << "Server started on port " << PORT << "\n"; // Log server start

    while (true) {                                         // Run server loop forever
        sockaddr_in clientAddress;                         // Structure for client info
        int clientSize = sizeof(clientAddress);            // Size of client address structure
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientSize); // Accept connection
        if (clientSocket != INVALID_SOCKET) {              // If valid connection
            EnterCriticalSection(&cs);                     // Lock access to clients list
            clients.push_back(clientSocket);               // Add new client to list
            LeaveCriticalSection(&cs);                     // Unlock after modification

            CreateThread(NULL, 0, ClientHandler, &clientSocket, 0, NULL); // Start new thread for client
        }
    }

    closesocket(serverSocket);                            // Close server socket (unreachable)
    DeleteCriticalSection(&cs);                           // Delete critical section object
    WSACleanup();                                         // Cleanup Winsock resources
    return 0;                                             // Exit main
}
