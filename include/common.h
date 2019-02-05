#pragma once

#include <string>

#include <arpa/inet.h>


constexpr sa_family_t IPv4 = AF_INET;
constexpr sa_family_t IPv6 = AF_INET6;

constexpr sa_family_t TCP = SOCK_STREAM;
constexpr sa_family_t UDP = SOCK_DGRAM;


sockaddr_in Address(const std::string& address, int port);
void Send(int socket, std::string message);
std::string Receive(int socket, size_t max_size_to_receive);
