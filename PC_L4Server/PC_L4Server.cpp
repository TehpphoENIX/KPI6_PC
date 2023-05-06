#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define DEFAULT_PORT "33003"

#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <Windows.h>

#include <iostream>
#include <vector>
#include <thread>
#include <string>

using std::cout, std::cerr, std::endl;

void clientThreadEx(SOCKET clientSocket, std::string clientId)
{
    cout << "Thread \"" << std::this_thread::get_id() << "\" handling client \"" << clientId << "\"" << endl;
}

int main()
{
    int iResult;
    WSADATA wsaData;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "WSAStartup failed: " << iResult << endl;
        return 1;
    }

    // Resolve the local address and port to be used by the server
    struct addrinfo* result = nullptr, * ptr = nullptr, hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cerr << "getaddrinfo failed: " << iResult << endl;
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cerr << "Error at socket(): " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cerr << "bind failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    {
        std::string host, port;
        host.resize(NI_MAXHOST);
        port.resize(NI_MAXSERV);
        getnameinfo(result->ai_addr, result->ai_addrlen, host.data(), host.size(), port.data(), port.size(), NI_NUMERICSERV | NI_NUMERICHOST);
        cout << "Server started at " << host << ":" << port << endl;
    }
    freeaddrinfo(result);

    // Listen to socket
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    SOCKET ClientSocket = INVALID_SOCKET;
    sockaddr clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    std::vector<std::thread> clientThreads;
    while (ClientSocket = accept(ListenSocket, &clientAddr, &clientAddrSize))
    {
        std::string host, port, clientId;
        host.resize(NI_MAXHOST);
        port.resize(NI_MAXSERV);
        getnameinfo(&clientAddr, clientAddrSize, host.data(), host.size(), port.data(), port.size(), NI_NUMERICSERV | NI_NUMERICHOST);
        clientId = host + ":" + port;
        cout << clientId << " connected" << endl;
        clientThreads.emplace_back(std::thread(clientThreadEx,ClientSocket,clientId));
    }

    for (std::thread& th : clientThreads)
        th.join();

    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}