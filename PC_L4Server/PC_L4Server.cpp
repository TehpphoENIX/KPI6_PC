#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define DEFAULT_PORT "33003"

#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <Windows.h>

#include <signal.h>

#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <array>
#include <sstream>
#include <future>
#include <random>

#include "Request.h"
#include "BitDecypher.h"

using std::cout, std::cerr, std::endl, std::stringstream, std::thread;

SOCKET ListenSocket = INVALID_SOCKET;
std::vector<std::thread> clientThreads;

bool serverActive = true;

void signalInterruptHandler(int signum)
{
    char answer;
    cout << "\nShut down server? (Y?N):";
    std::cin >> answer;
    if (std::tolower(answer) == 'y')
    {
        cout << "Server shutting down" << std::endl;

        serverActive = false;
        for (std::thread& th : clientThreads)
        {
            ResumeThread(th.native_handle());
            th.detach();
        }
        closesocket(ListenSocket);
        WSACleanup();

        exit(signum);
    }
}

int intRand(const int& min, const int& max) {
    static thread_local std::mt19937 generator(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

void workerThredEx(const matrix_t matrix, const int32_t min, const int32_t max, std::shared_ptr<std::promise<matrix_t>> promise, unsigned int numberOfThreads = 4) 
{
    matrix_t Array = matrix;

    const unsigned int arrayDimension = Array.size();

    std::vector<std::thread> threads;
    {
        const auto numberOfThreadsCalc = min(numberOfThreads, arrayDimension * arrayDimension);
        const double randOpPerThread = (double)(arrayDimension * arrayDimension) / numberOfThreads;
        for (size_t i = 0; i < numberOfThreads; i++)
        {
            unsigned int start = i * randOpPerThread;
            unsigned int end = (i + 1) * randOpPerThread - 1;
            threads.push_back(thread([&, start, end]
                {
                    for (size_t i = start; i <= end; i++)
                    {
                        if (i / arrayDimension != i % arrayDimension) {
                            Array[i / arrayDimension][i % arrayDimension] = intRand(min, max);
                        }
                        else
                        {
                            Array[i / arrayDimension][i % arrayDimension] = 0;
                        }
                    }
                }));
        }
    }
    for (thread& th : threads)
    {
        th.join();
    }

    threads.clear();
    {
        const auto numberOfThreadsCalc = min(numberOfThreads, arrayDimension);
        const double calcOpPerThread = (double)arrayDimension / numberOfThreadsCalc;
        for (size_t i = 0; i < numberOfThreadsCalc; i++)
        {
            unsigned int start = i * calcOpPerThread;
            unsigned int end = (i + 1) * calcOpPerThread - 1;
            threads.push_back(thread([&, start, end]
                {
                    for (size_t i = start; i <= end; i++)
                    {
                        for (size_t j = 0; j < arrayDimension; j++)
                        {
                            if (j != i) {
                                Array[i][i] += Array[i][j];
                            }
                        }
                    }
                }));
        }
    }
    for (thread& th : threads)
    {
        th.join();
    }

    promise->set_value(Array);
}

void clientThreadEx(SOCKET clientSocket, std::string clientId)
{
    {
        stringstream message;
        message << "Thread \"" << std::this_thread::get_id() << "\" handling client \"" << clientId << "\"";
        cout << message.str() << endl;
    }
    std::array<char, 4> sizeRecieveBuffer;
    uint32_t messageSize = 0;//size must be equal to sizeRecieveBuffer (note b, B)!
    std::vector<char> messageBuffer;

    std::unique_ptr<matrix_t> matrix;
    std::unique_ptr<int32_t> min, max;
    std::unique_ptr<std::pair<std::thread, std::future<matrix_t>>> worker;
    while(serverActive){
        // read first 4 bytes (size of message)
        int iResult = recv(clientSocket, sizeRecieveBuffer.data(), sizeRecieveBuffer.size(), MSG_PEEK);
        if (iResult < 0)
        {
            stringstream message;
            message << "(" << std::this_thread::get_id() << ") Encountered error: " << WSAGetLastError();
            cerr << message.str() << endl;
        }
        else if (iResult == 0)
        {
            stringstream message;
            message << "(" << std::this_thread::get_id() << ") Connection closed.";
            cout << message.str() << endl;
            return;
        }
        else
        {
            // read full message
            bitsToInt<uint32_t>(messageSize, (const unsigned char*)sizeRecieveBuffer.data(), false);
            messageBuffer.resize(messageSize);
            iResult = recv(clientSocket, messageBuffer.data(), messageBuffer.size(), 0);

            Request req = Request::readRequest(messageBuffer);

            auto data = req.data();

            //log
            {
                stringstream message;
                message << "(" << std::this_thread::get_id() << ") Recieved:\n";
                message << req;
                cout << message.str() << endl;
            }

            //handle request
            Request response;
            switch (req.command())
            {
            case COMMAND_TYPES::upload:
                matrix = std::make_unique<std::vector<std::vector<int>>>(req.data());
                min = std::make_unique <int32_t>(req.getMin());
                max = std::make_unique<int32_t>(req.getMax());
                response = responseDataAccepted;
                break;
            case COMMAND_TYPES::start:
                if (
                    matrix.get() != nullptr &&
                    min.get() != nullptr &&
                    max.get() != nullptr &&
                    *min.get() < *max.get()
                    )
                {
                    auto ptr = std::make_shared<std::promise<matrix_t>>();
                    worker = std::make_unique<std::pair<std::thread, std::future<matrix_t>>>(
                        std::thread(workerThredEx, *matrix.get(), *min.get(), *max.get(), ptr, 4), 
                        ptr.get()->get_future());
                    response = responseProcessing;
                }
                else 
                {
                    response = responseError;
                }
                break;
            case COMMAND_TYPES::status:
                if (worker.get() == nullptr)
                {
                    response = responseIdle;
                }
                else if (worker.get()->second.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready)
                {
                    auto result = worker.get()->second.get();
                    worker.get()->first.join();
                    worker.release();
                    response = responseSuccess(result);
                }
                else
                {
                    response = responseProcessing;
                }
                break;
            default:
                response = responseError;
            }

            {
                stringstream message;
                message << "(" << std::this_thread::get_id() << ") Sending:\n";
                message << response;
                cout << message.str() << endl;
            }

            //send response
            std::vector<char> rm = response.to_bytes();
            send(clientSocket, rm.data(), rm.size(), 0);

            //clear buffers
            sizeRecieveBuffer.fill('\0');
            messageBuffer.clear();
        }
    }

    if (worker.get() != nullptr && worker.get()->first.joinable())
    {
        worker.get()->first.detach();
    }
    closesocket(clientSocket);
}

int main()
{
    signal(SIGINT, signalInterruptHandler);

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
    while ((ClientSocket = accept(ListenSocket, &clientAddr, &clientAddrSize)) && serverActive)
    {
        std::string host, port, clientId;
        host.resize(NI_MAXHOST);
        port.resize(NI_MAXSERV);
        getnameinfo(&clientAddr, clientAddrSize, host.data(), host.size(), port.data(), port.size(), NI_NUMERICSERV | NI_NUMERICHOST);
        clientId = host + ":" + port;
        stringstream message;
        message << clientId << " connected";
        cout << message.str() << endl;
        clientThreads.emplace_back(std::thread(clientThreadEx,ClientSocket,clientId));
    }

    for (std::thread& th : clientThreads)
        th.detach();

    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}