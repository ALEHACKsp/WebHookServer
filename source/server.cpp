#include "server.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <arpa/inet.h>
#include <unistd.h>

#include "common.h"


int ConnectServer(const std::string& address, int port, int max_number_of_clients)
{
    // The steps involved in establishing a socket on the server side are as follows:
    //
    //     1. Create a socket with the socket() system call.
    //     2. Bind the socket to an address using the bind() system call. For a server socket on the Internet,
    //        an address consists of a port number on the host machine.
    //     3. Listen for connections with the listen() system call.
    //     4. Accept a connection with the accept() system call. This call typically blocks until a client connects
    //        with the server.
    //     5. Send and receive data.

    int success;

    // http://man7.org/linux/man-pages/man2/socket.2.html
    //     socket(domain, type, protocol) creates an endpoint for communication and returns a file descriptor that
    //     refers to that endpoint
    //         domain: most commonly AF_INET (IPv4) or AF_INET6 (IPv6)
    //         type: most commonly SOCK_STREAM or SOCK_DGRAM
    //         protocol: 0 unless more protocols in the protocol family exist.
    int listen_socket = socket(IPv4, TCP, 0);
    if (listen_socket == -1)
        throw std::runtime_error("Couldn't create socket.");


    // http://man7.org/linux/man-pages/man2/bind.2.html
    //     bind(socket, address, size) assigns/binds the address to the socket.
    //
    sockaddr_in server_address = Address(address, port);
    success = bind(listen_socket, (sockaddr*)&server_address, sizeof(server_address));
    if (success == -1)
        throw std::runtime_error("Couldn't bind socket. Errno: " + std::to_string(errno) + ".");


    // http://man7.org/linux/man-pages/man2/listen.2.html
    //     listen(socket, backlog) marks the socket as a passive socket that will be used to accept incoming connection.
    //        socket: socket of type SOCK_STREAM or SOCK_SEQPACKET.
    //        backlog: the maximum length to which the queue of pending connections for socket may grow.
    success = listen(listen_socket, max_number_of_clients);
    if (success == -1)
        throw std::runtime_error("Can't listen to socket. Errno: " + std::to_string(errno) + ".");


    // // http://man7.org/linux/man-pages/man2/accept.2.html
    // //     accept(socket, address, size, flags)
    // //         socket: socket of type SOCK_STREAM or SOCK_SEQPACKET.
    // sockaddr_in client_address{};
    // socklen_t client_address_size = sizeof(client_address);
    // int client_socket = accept(listen_socket, (sockaddr*)&client_address, &client_address_size);
    // if (client_socket == -1)
    //     throw std::runtime_error("Couldn't accept request from client. Errno: " + std::to_string(errno) + ".");
    //
    //
    // close(listen_socket);
    return listen_socket;
}



int HandleClients(int listening_socket, const std::function<void(int, int)>& operation)
{
    unsigned counter = 0;

    while (true)
    {
        // http://man7.org/linux/man-pages/man2/accept.2.html
        //     accept(socket, address, size, flags)
        //         socket: socket of type SOCK_STREAM or SOCK_SEQPACKET.
        sockaddr_in client_address{};
        socklen_t client_address_size = sizeof(client_address);
        int client_socket = accept(listening_socket, (sockaddr*)&client_address, &client_address_size);
        if (client_socket == -1)
            throw std::runtime_error("Couldn't accept request from client. Errno: " + std::to_string(errno) + ".");

        // http://man7.org/linux/man-pages/man2/fork.2.html
        int pid = fork();

        if (pid < 0)
            throw std::runtime_error("Couldn't create fork.");
        else if (pid != 0)
            // Parent process (pid is the pid of child process).
            close(client_socket);
        else
        {
            // Child process.
            std::cout << "Client " << ++counter << " joined." << std::endl;
            close(listening_socket);
            operation(client_socket, counter);
            exit(0);
        }
    }
}
















