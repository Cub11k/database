#ifndef DATABASEDIMA__CLIENT_H_
#define DATABASEDIMA__CLIENT_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <csignal>
#include <sstream>

#include "db.h"

namespace db::client {
    class Client {
    private:
        int m_socket{};
        sockaddr_in m_server{};
        hostent *m_hp{};
    public:
        Client(const std::string &address, int port);

        void work();
    };
}

#endif //DATABASEDIMA__CLIENT_H_
