#pragma once

#include <string>
#include <functional>

int ConnectServer(const std::string& address, int port, int max_number_of_clients = 1);
int HandleClients(int listening_socket, const std::function<void(int, int)>& operation);