#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <cstring>
#include <stdexcept>

#include "common.h"
#include "server.h"
#include "client.h"



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
    std::string starter = "Client " + std::to_string(id) + ": ";

    while (true)
    {
        std::string message;
        std::getline(std::cin, message);

        if (!message.empty())
            Send(socket, starter+message);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


void Communication(int socket, int id)
{
    int max_transmission_size = 1000;

    std::thread talk_thread(Talk, socket, id);
    std::thread listen_thread(Listen, socket, id, max_transmission_size);

    talk_thread.join();
    listen_thread.join();

    std::cout << "Exiting program." << std::endl;
}


void RunServer(std::string address, int port)
{
    std::cout << "Listening for clients on port " << port << "." << std::endl;
    int socket = ConnectServer(address, port);
    std::cout << "Calling HandleClients from server." << std::endl;
    HandleClients(socket, Communication);
}


void RunClient(std::string address, int port)
{
    std::cout << "Trying to connect to server " << address << " on port " << port << "." << std::endl;
    int socket = ConnectClient(address, port);
    std::string id = Receive(socket, 100);
    std::cout << "Calling communication from client." << std::endl;
    Communication(socket, std::atoi(id.c_str()));
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
    {
        std::cout << "Connection established!\n";
        std::cout << "Welcome!\n\n"
                     "* Type anything and press Enter to send.\n"
                     "* You can type ! followed by a bash command. A useful one is !clear.\n"
                     "* Press ctrl-c to quit the application.\n"
                  << std::endl;
        RunClient(options.address, options.port);
    }

}