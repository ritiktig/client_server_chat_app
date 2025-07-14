
#include <winsock2.h>         // Winsock2 for networking
#include <ws2tcpip.h>         // For additional socket functions (not used much here)
#include <iostream>           // For input/output
#include <windows.h>          // For Windows-specific features like threads

#pragma comment(lib, "ws2_32.lib")  // Link against Winsock library

#define PORT 9001                    // Port number for the server
#define SERVER_IP "127.0.0.1"        // Server IP address (localhost)

// Thread function to receive messages from the server
DWORD WINAPI receiveMessages(LPVOID lpParam) {
    SOCKET sock = *(SOCKET*)lpParam;     // Extract the socket from void pointer
    char buffer[1024];                   // Buffer to store received messages

    while (true) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0); // Receive message
        if (bytesReceived > 0) {            // If message received
            buffer[bytesReceived] = '\0';  // Null-terminate the message
            std::cout << "\nFriend: " << buffer << "\nYou: "; // Print message
            fflush(stdout);                // Flush output buffer for clean display
        }
    }
    return 0; // End thread
}

int main() {
    WSADATA wsaData;                                 // Structure to hold Winsock info
    WSAStartup(MAKEWORD(2,2), &wsaData);             // Initialize Winsock with version 2.2

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);   // Create TCP socket

    sockaddr_in serverAddress;                       // Structure for server address
    serverAddress.sin_family = AF_INET;              // Use IPv4
    serverAddress.sin_port = htons(PORT);            // Set port in network byte order
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP); // Convert IP string to binary form

    // Attempt to connect to the server
    if (connect(sock, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cout << "Failed to connect to server\n"; // If connection fails
        return 1;                                     // Exit with error
    }

    std::cout << "Connected to server!\n";            // Connection success message

    // Create a separate thread to receive messages
    CreateThread(NULL, 0, receiveMessages, &sock, 0, NULL);

    std::string input;                                // To store user input
    while (true) {
        std::cout << "You: ";
        std::getline(std::cin, input);               // Get full line of input from user
        send(sock, input.c_str(), input.length(), 0); // Send input to server
    }

    closesocket(sock);                               // Close the socket
    WSACleanup();                                    // Cleanup Winsock
    return 0;                                        // Exit program
}

