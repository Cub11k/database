#include "client.h"

static volatile sig_atomic_t interrupted = false;

db::client::Client::Client(const std::string &address, int port)
{
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == -1) {
        perror("Socket creation failed");
        throw std::exception();
    }
    m_server.sin_family = AF_INET;
    if ((m_hp = gethostbyname(address.c_str())) == nullptr) {
        perror("Can't find host");
        throw std::exception();
    }
    bcopy(m_hp->h_addr, &m_server.sin_addr, m_hp->h_length);
    m_server.sin_port = htons(port);
}

void db::client::Client::work()
{
    if (connect(m_socket, reinterpret_cast<sockaddr *>(&m_server), sizeof(m_server)) == -1) {
        perror("Connection failed");
        close(m_socket);
        throw std::exception();
    }
    while (!interrupted) {
        if (isatty(STDIN_FILENO)) {
            std::cout << ">>> ";
        }
        std::string msg{};
        bool end = false;
        std::string tmp_str{};
        while (!end && !interrupted && std::getline(std::cin, tmp_str, '\n')) {
            if (msg.empty() && tmp_str == "exit") {
                close(m_socket);
                return;
            }
            if (tmp_str.ends_with("END") && (tmp_str.size() == 3 || isspace(tmp_str[tmp_str.size() - 4]))) {
                end = true;
            } else if (isatty(STDIN_FILENO)) {
                std::cout << "> ";
            }
            tmp_str.push_back('\n');
            msg += tmp_str;
            tmp_str = {};
        }

        if (!interrupted) {
            size_t len = msg.size();
            if (write(m_socket, &len, sizeof(len)) == sizeof(len)) {
                ssize_t sent{};
                while (sent != (ssize_t) len) {
                    ssize_t tmp = write(m_socket, &msg.c_str()[sent], len - sent);
                    if (tmp != -1) {
                        sent += tmp;
                    }
                }
                if (recv(m_socket, &len, sizeof(len), MSG_WAITALL) == sizeof(len)) {
                    if (!interrupted) {
                        // receive message by parts of fixed size
                        ssize_t received{};
                        while (received < (ssize_t) len) {
                            auto buf = new char[db::Database::buf_size()];
                            for (size_t i = 0; i < db::Database::buf_size(); ++i) {
                                buf[i] = 0;
                            }
                            ssize_t tmp = recv(m_socket, buf, std::min(db::Database::buf_size(), len - received), MSG_WAITALL);
                            std::cout << std::string(buf, tmp);
                            std::cout.flush();
                            delete[] buf;
                            received += (ssize_t) std::min(db::Database::buf_size(), len - received);
                        }
                        std::cout << std::endl;
                    }
                } else {
                    std::cout << "Response not received" << std::endl;
                }
            } else {
                std::cout << "Message was not sent" << std::endl;
            }
        }
    }
    close(m_socket);
}

void signal_handler(int signal)
{
    interrupted = true;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    if (argc < 3) {
        std::cout << "Not enough arguments\nAddress and port required" << std::endl;
        return 1;
    }
    std::istringstream stream(argv[2]);
    int port{};
    if (!(stream >> port)) {
        std::cout << "Invalid port" << std::endl;
        return 1;
    }
    try {
        db::client::Client client(argv[1], port);
        client.work();
    } catch (const std::exception &err) {
        return 1;
    }
    return 0;
}