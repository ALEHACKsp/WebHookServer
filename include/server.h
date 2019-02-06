#pragma once

#include <string>
#include <functional>

int ConnectServer(const std::string& address, int port, int max_number_of_clients = 1);
int DelegateClients(
        int listening_socket,
        const std::function<void(int, int)>& start_up   = [](int, int){},
        const std::function<void(int, int)>& running    = [](int, int){},
        const std::function<void(int, int)>& tear_down  = [](int, int){}
);