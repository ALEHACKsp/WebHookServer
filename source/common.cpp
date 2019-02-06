#include "common.h"

#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <unistd.h>



sockaddr_in Address(const std::string& address, int port)
{
    // http://man7.org/linux/man-pages/man7/ip.7.html
    // struct sockaddr_in {
    //     sa_family_t    sin_family; /* address family: AF_INET */
    //     in_port_t      sin_port;   /* port in network byte order */
    //     struct in_addr sin_addr;   /* internet address */
    // };
    // sin_family is always set to AF_INET.
    sockaddr_in socket_address{};
    socket_address.sin_family = IPv4;
    socket_address.sin_port = htons(port);  // htons - Host to Network Short. Flip endianness to match machine.
    int success = inet_pton(IPv4, address.c_str(), &socket_address.sin_addr);  // Pointer (to String) to Number.
    if (success <= 0)
        throw std::runtime_error("Invalid address: " + address);

    return socket_address;
}


void Send(int socket, std::string message)
{
    ssize_t bytes_written = write(socket, message.c_str(), message.size());
    if (bytes_written == -1)
        throw std::runtime_error("Couldn't write to socket.");
}


std::string Receive(int socket, size_t max_size_to_receive)
{
    char* message = new char[max_size_to_receive];

    // Wait for message
    // http://man7.org/linux/man-pages/man2/recvmsg.2.html
    //     recv(socket, buffer, size, flags)
    //          socket: any socket.
    //          buffer: array to fill with the message.
    //          size: the size of the buffer.
    //          flags: options.
    ssize_t bytes_received = recv(socket, message, max_size_to_receive, 0);
    if (bytes_received == -1)
        throw std::runtime_error("Issue with connection.");
    else if (bytes_received == 0)
        throw std::runtime_error("The client disconnected.");
    return message;
}