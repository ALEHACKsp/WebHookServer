#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cerrno>

#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>



constexpr int MAX_NUMBER_OF_CLIENTS = 10;
std::atomic<bool> connected;

// IPv4_FAMILY:  IPv4
// IPv4_FAMILY6: IPv6
sa_family_t IPv4_FAMILY = AF_INET;


sockaddr_in socket_address(const std::string& address, int port)
{
    // http://man7.org/linux/man-pages/man7/ip.7.html
    // struct sockaddr_in {
    //     sa_family_t    sin_family; /* address family: AF_INET */
    //     in_port_t      sin_port;   /* port in network byte order */
    //     struct in_addr sin_addr;   /* internet address */
    // };
    // sin_family is always set to AF_INET.
    sockaddr_in socket_address{};
    socket_address.sin_family = IPv4_FAMILY;
    socket_address.sin_port = htons(port);  // htons - Host to Network Short. Flip endianness to match machine.
    int success = inet_pton(IPv4_FAMILY, address.c_str(), &socket_address.sin_addr);  // Pointer (to String) to Number.
    if (success <= 0)
        throw std::runtime_error("Invalid address: " + address);
    return socket_address;
}


int listen_for_clients(const std::string& address, int port, int max_number_of_clients = MAX_NUMBER_OF_CLIENTS)
{
    int success;

    // tcp_socket = socket(IPv4_FAMILY, SOCK_STREAM, 0);
    // udp_socket = socket(IPv4_FAMILY, SOCK_DGRAM, 0);
    // raw_socket = socket(IPv4_FAMILY, SOCK_RAW, protocol);

    // http://man7.org/linux/man-pages/man2/socket.2.html
    int listen_socket = socket(IPv4_FAMILY, SOCK_STREAM, 0);
    if (listen_socket == -1)
    {
        std::cerr << "Couldn't create socket." << std::endl;
        return -1;
    }

    sockaddr_in server_address = socket_address(address, port);

    // http://man7.org/linux/man-pages/man2/bind.2.html
    success = bind(listen_socket, (sockaddr*)&server_address, sizeof(server_address));
    if (success == -1)
    {
        std::cerr << "Couldn't bind socket. Errno: " << errno << std::endl;
        return -1;
    }

    // http://man7.org/linux/man-pages/man2/listen.2.html
    success = listen(listen_socket, max_number_of_clients);
    if (success == -1)
    {
        std::cerr << "Can't listen to socket. Errno: " << errno << std::endl;
        return -1;
    }

    sockaddr_in client_address{};
    socklen_t client_address_size = sizeof(client_address);
    int client_socket = accept(listen_socket, (sockaddr*)&client_address, &client_address_size);
    if (client_socket == -1)
    {
        std::cerr << "Couldn't accept request from client. Errno: " << errno << std::endl;
        return -1;
    }

    close(listen_socket);

    return client_socket;
}


int connect_to_server(const std::string& address, int port)
{
    // tcp_socket = socket(IPv4_FAMILY, SOCK_STREAM, 0);
    // udp_socket = socket(IPv4_FAMILY, SOCK_DGRAM, 0);
    // raw_socket = socket(IPv4_FAMILY, SOCK_RAW, protocol);

    // http://man7.org/linux/man-pages/man2/socket.2.html
    int server_socket = socket(IPv4_FAMILY, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Couldn't create socket." << std::endl;
        return -1;
    }

    sockaddr_in server_address = socket_address(address, port);

    int success = connect(server_socket, (sockaddr*)&server_address, sizeof(server_address));
    if (success == -1)
    {
        std::cerr << "Couldn't connect to server. Errno: " << errno << std::endl;
        return -1;
    }

    return server_socket;
}


int send_via_socket(int socket, size_t max_size_to_send)
{
    std::string buffer;
    buffer.reserve(max_size_to_send);

    while (true)
    {
        if (std::getline(std::cin, buffer))
        {
            if (buffer[0] == '!')
            {
                char command[buffer.size() - 1];
                strcpy(command, &buffer[1]);
                system(command);
            }
            else if (buffer[0] == '\n')
            {
                continue;
            }
            else
            {
                ssize_t bytes_written = write(socket, buffer.c_str(), max_size_to_send);
                if (bytes_written == -1)
                {
                    std::cerr << "Something went wrong!" << std::endl;
                    connected = false;
                    return errno;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

}

int receive_via_socket(int socket, size_t max_size_to_receive)
{
    char buffer[max_size_to_receive];

    while (true)
    {
        memset(buffer, 0, max_size_to_receive);
        // Wait for message
        // http://man7.org/linux/man-pages/man2/recvmsg.2.html
        ssize_t bytes_received = read(socket, buffer, max_size_to_receive);
        if (bytes_received == -1)
        {
            std::cerr << "Issue with connection." << std::endl;
            connected = false;
            return errno;
        }
        else if (bytes_received == 0)
        {
            std::cout << "The client disconnected." << std::endl;
            connected = false;
            return errno;
        }
        else if (bytes_received > 0)
        {
            std::cout << "Other: " << buffer << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}


struct Options
{
    std::string address = "";
    int port = -1;
    bool server = false;
};


Options parse_command_arguments(int argc, char* argv[])
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
    try
    {
        options = parse_command_arguments(argc, argv);
    }
    catch (const std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
        return -1;
    }

    int socket;
    if (options.server)
    {
        std::cout << "Listening for clients on port " << options.port << "." << std::endl;
        socket = listen_for_clients(options.address, options.port);
    }
    else
    {
        std::cout << "Trying to connect to server " << options.address << " on port " << options.port << "." << std::endl;
        socket = connect_to_server(options.address, options.port);
    }

    std::cout << "Connection established!\n" << std::endl;
    std::cout << "Welcome!\n\n"
                 "* Type anything and press Enter to send.\n"
                 "* You can type ! followed by a bash command. A useful one is !clear.\n"
                 "* Press ctrl-c to quit the application.\n"
              << std::endl;
    connected = true;

    int max_transmission_size = 1000;

    std::thread sending_thread(send_via_socket, socket, max_transmission_size);
    std::thread receiving_thread(receive_via_socket, socket, max_transmission_size);

    while (connected)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    sending_thread.join();
    receiving_thread.join();

    std::cout << "Exiting program." << std::endl;
    close(socket);

    return 0;
}