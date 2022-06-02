#ifndef DATABASEDIMA__SERVER_H_
#define DATABASEDIMA__SERVER_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include <ctime>

#include "db.h"

namespace db::server {
    class Server {
    private:
        constexpr static char too_long_msg[] = "Message is too long, it will be skipped";
        constexpr static char read_failed_msg[] = "Read failed";

        enum Constants {
            PORT = 8080
        };

        int m_opt = 1;
        int m_master_socket{};
        std::vector<int> m_client_sockets{};
        socklen_t m_address_len{};
        sockaddr_in m_address{};

        db::Database m_db{};
    public:
        Server(int port = PORT);

        void work();
    };
}

#endif //DATABASEDIMA__SERVER_H_
