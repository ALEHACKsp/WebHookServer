#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <cstring>
#include <stdexcept>

#include "common.h"
#include "server.h"
#include "client.h"


std::atomic<bool> connected;

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


void Listen(int socket, size_t max_transmission_size)
{
    while (connected)
    {
        std::string message = Receive(socket, max_transmission_size);
        std::cout << ">>> " << message << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Talk(int socket)
{
    while (connected)
    {
        std::string message;
        std::getline(std::cin, message);

        if (!message.empty())
            Send(socket, message);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


int main(int argc, char* argv[])
{
    // -server port
    // -client address port

    Options options;
    options = ParseCommandArguments(argc, argv);

    int socket;
    if (options.server)
    {
        std::cout << "Listening for clients on port " << options.port << "." << std::endl;
        socket = ConnectServer(options.address, options.port);
    }
    else
    {
        std::cout << "Trying to connect to server " << options.address << " on port " << options.port << "." << std::endl;
        socket = ConnectClient(options.address, options.port);
    }

    std::cout << "Connection established!\n" << std::endl;
    std::cout << "Welcome!\n\n"
                 "* Type anything and press Enter to send.\n"
                 "* You can type ! followed by a bash command. A useful one is !clear.\n"
                 "* Press ctrl-c to quit the application.\n"
              << std::endl;
    connected = true;

    int max_transmission_size = 1000;

    std::thread talk_thread(Talk, socket);
    std::thread listen_thread(Listen, socket, max_transmission_size);

    while (connected)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    talk_thread.join();
    listen_thread.join();

    std::cout << "Exiting program." << std::endl;
    return 0;
}