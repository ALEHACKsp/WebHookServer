#include "client.h"

#include <string>
#include <cerrno>
#include <stdexcept>

#include "common.h"


int ConnectClient(const std::string& address, int port)
{
    // http://man7.org/linux/man-pages/man2/socket.2.html
    int server_socket = socket(IPv4, TCP, 0);
    if (server_socket == -1)
        throw std::runtime_error("Couldn't create socket.");

    sockaddr_in server_address = Address(address, port);
    int success = connect(server_socket, (sockaddr*)&server_address, sizeof(server_address));
    if (success == -1)
        throw std::runtime_error("Couldn't connect to server. Errno: " + std::to_string(errno) + ".");

    return server_socket;
}

