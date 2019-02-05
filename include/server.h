#pragma once

#include <string>

int ConnectServer(const std::string& address, int port, int max_number_of_clients = 1);