#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <cstring>
#include <stdexcept>

#include "common.h"
#include "server.h"
#include "client.h"


// ----  ----
void Listen(int socket, int id, size_t max_transmission_size)
{
    std::string starter = "Client " + std::to_string(id) + ": ";

    while (true)
    {
        std::string message = Receive(socket, max_transmission_size);
        std::cout << starter << message << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Talk(int socket, int id)
{
    while (true)
    {
        std::string message;
        std::getline(std::cin, message);

        if (!message.empty())
            Send(socket, message);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}



// ---- SERVER ----

void StartUp(int socket, int id)
{
    std::cout << "Starting up client " << id << std::endl;
    Send(socket, std::to_string(id));
}

void Running(int socket, int id)
{
    std::cout << "Running client " << id << std::endl;
    std::string welcome_message = "Connection established!";
    Send(socket, welcome_message);

    int max_transmission_size = 1000;
    Listen(socket, id, max_transmission_size);
}

void TearDown(int socket, int id)
{
    std::cout << "Tearing down client " << id << std::endl;
}


void RunServer(std::string address, int port)
{
    std::cout << "Listening for clients on port " << port << "." << std::endl;
    int socket = ConnectServer(address, port);
    DelegateClients(socket, StartUp, Running, TearDown);
}



// ---- CLIENT ----

void RunClient(std::string address, int port)
{
    std::cout << "Trying to connect to server " << address << " on port " << port << "." << std::endl;
    int socket = ConnectClient(address, port);

    // Start-up
    std::string id = Receive(socket, 100);
    std::cout << "Connected with id " << id << "." << std::endl;

    // Running
    Talk(socket, std::atoi(id.c_str()));

    // Tear-down.

}


// ---- STARTUP ----
struct Options
{
    std::string address = "";
    int port = -1;
    bool server = false;
};


Options ParseCommandArguments(int argc, char* argv[])
{
    // -server port
    // -client address port
    Options options;

    if (argc < 2)
    {
        throw std::runtime_error("Usage 1: client <address> <port>\nUsage 2: server <port>");
    }
    else if (argc == 4 && std::strcmp(argv[1], "client") == 0)
    {
        options.address = argv[2];
        options.port = std::atoi(argv[3]);
        options.server = false;
    }
    else if (argc == 3 && std::strcmp(argv[1], "server") == 0)
    {
        options.address = "0.0.0.0";
        options.port = std::atoi(argv[2]);
        options.server = true;
    }
    else
    {
        throw std::runtime_error("Usage 1: client <address> <port>\nUsage 2: server <port>");
    }

    return options;
}


int main(int argc, char* argv[])
{
    // -server port
    // -client address port

    Options options;
    options = ParseCommandArguments(argc, argv);

    if (options.server)
        RunServer(options.address, options.port);
    else
        RunClient(options.address, options.port);

}