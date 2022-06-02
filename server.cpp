#include "server.h"

static volatile sig_atomic_t interrupted = false;

db::server::Server::Server(int port)
{
    if ((m_master_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        throw std::exception();
    }
    if (setsockopt(m_master_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&m_opt), sizeof(m_opt)) == -1) {
        perror("Socket options setup failed");
        close(m_master_socket);
        throw std::exception();
    }
    m_address.sin_family = AF_INET;
    m_address.sin_addr.s_addr = INADDR_ANY;
    m_address.sin_port = htons(port);
    m_address_len = sizeof(m_address);
    if (bind(m_master_socket, reinterpret_cast<sockaddr *>(&m_address), m_address_len) < 0) {
        perror("Bind failed");
        close(m_master_socket);
        throw std::exception();
    }
    if (listen(m_master_socket, 10) == -1) {
        perror("Listen failed");
        close(m_master_socket);
        throw std::exception();
    }
    m_client_sockets.push_back(m_master_socket);
}

void db::server::Server::work()
{
    while (!interrupted) {
        fd_set read_fds;
        FD_ZERO(&read_fds);

        FD_SET(0, &read_fds);
        FD_SET(m_master_socket, &read_fds);
        for (auto &client : m_client_sockets) {
            FD_SET(client, &read_fds);
        }
        int max_fd = *std::max_element(m_client_sockets.begin(), m_client_sockets.end());

        timeval tv{
            .tv_sec = 3,
            .tv_usec = 0
        };

        int select_return = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);

        if (select_return == -1) {
            perror("Select failed");
        } else if (select_return > 0) {
            if (FD_ISSET(0, &read_fds)) {
                // data from keyboard
                std::string buf{};
                std::getline(std::cin, buf, '\n');
                if (buf == "exit") {
                    close(m_master_socket);
                    for (auto &client : m_client_sockets) {
                        close(client);
                    }
                    return;
                }
                std::cout << "Unknown command" << std::endl;
            }

            for (size_t i = 1; i < m_client_sockets.size(); i++) {
                if (FD_ISSET(m_client_sockets[i], &read_fds)) {
                    size_t len{};
                    ssize_t len_received = recv(m_client_sockets[i], &len, sizeof(len), MSG_WAITALL);
                    if (len_received == sizeof(len)) {
                        std::cout << "New message from " << m_client_sockets[i] << std::endl;
                        // success
                        if (len < 100000) {
                            auto buf = new char[len + 1];
                            for (size_t j = 0; j < len + 1; ++j) {
                                buf[j] = 0;
                            }
                            if (recv(m_client_sockets[i], buf, len, MSG_WAITALL) == (ssize_t) len) { // success
                                // replace response with vector of strings of fixed size (for example 1KB) and send one by one
                                std::vector<std::string> response = m_db.process_query(buf, m_client_sockets[i]);
                                len = response[0].size();
                                for (size_t j = 1; j < response.size(); ++j) {
                                    len += response[j].size();
                                }

                                if (write(m_client_sockets[i], &len, sizeof(len)) == sizeof(len)) {
                                    for (auto &str : response) {
                                        ssize_t sent{};
                                        while (sent != (ssize_t) str.size()) {
                                            ssize_t
                                                tmp = write(m_client_sockets[i], &str.c_str()[sent], str.size() - sent);
                                            if (tmp != -1) {
                                                sent += tmp;
                                            } else {
                                                break;
                                            }
                                        }
                                    }
                                } else {
                                    // send response to client failed
                                    perror("Write of response failed");
                                    len = sizeof(read_failed_msg);
                                    if (write(m_client_sockets[i], &len, sizeof(len)) == sizeof(len)) {
                                        ssize_t sent{};
                                        while (sent != (ssize_t) len) {
                                            ssize_t
                                                tmp = write(m_client_sockets[i], &read_failed_msg[sent], len - sent);
                                            if (tmp != -1) {
                                                sent += tmp;
                                            } else {
                                                break;
                                            }
                                        }
                                    } else {
                                        perror("Write about response error failed, skip");
                                    }
                                }
                            } else {
                                // reading message error
                                perror("Read of message failed");
                                len = sizeof(read_failed_msg);
                                if (write(m_client_sockets[i], &len, sizeof(len)) == sizeof(len)) {
                                    ssize_t sent{};
                                    while (sent != (ssize_t) len) {
                                        ssize_t tmp = write(m_client_sockets[i], &read_failed_msg[sent], len - sent);
                                        if (tmp != -1) {
                                            sent += tmp;
                                        } else {
                                            break;
                                        }
                                    }
                                } else {
                                    perror("Write about response error failed, skip");
                                }
                            }
                            delete[] buf;
                        } else {
                            // too long message
                            std::cout << "Message length is bigger than 100000, it will be skipped" << std::endl;
                            for (size_t j = 0; j < len; j += db::Database::buf_size()) {
                                char c[db::Database::buf_size()];
                                recv(m_client_sockets[i], c, std::min(db::Database::buf_size(), len - j), MSG_WAITALL);
                            }
                            len = sizeof(too_long_msg);
                            if (write(m_client_sockets[i], &len, sizeof(len)) == sizeof(len)) {
                                ssize_t sent{};
                                while (sent != (ssize_t) len) {
                                    ssize_t tmp = write(m_client_sockets[i], &too_long_msg[sent], len - sent);
                                    if (tmp != -1) {
                                        sent += tmp;
                                    } else {
                                        break;
                                    }
                                }
                            } else {
                                perror("Write about response error failed, skip");
                            }

                        }
                    } else if (len_received == 0) {
                        // disconnect
                        m_db.delete_client(m_client_sockets[i]);
                        close(m_client_sockets[i]);
                        std::cout << "Disconnected client on socket: " << m_client_sockets[i] << std::endl;
                        m_client_sockets.erase(std::find(m_client_sockets.begin(),
                                                         m_client_sockets.end(),
                                                         m_client_sockets[i]));
                        i--;
                    } else {
                        // reading len error
                        perror("Read of message failed");
                        len = sizeof(read_failed_msg);
                        if (write(m_client_sockets[i], &len, sizeof(len)) == sizeof(len)) {
                            ssize_t sent{};
                            while (sent != (ssize_t) len) {
                                ssize_t tmp = write(m_client_sockets[i], &read_failed_msg[sent], len - sent);
                                if (tmp != -1) {
                                    sent += tmp;
                                } else {
                                    break;
                                }
                            }
                        } else {
                            perror("Write about response error failed, skip");
                        }
                    }
                }
            }
            if (FD_ISSET(m_master_socket, &read_fds)) {
                // new client
                int new_client_sock = accept(m_master_socket, nullptr, nullptr);
                std::cout << "Connected client on socket: " << new_client_sock << std::endl;
                m_client_sockets.push_back(new_client_sock);
                m_db.insert_client(new_client_sock);
            }
        }
    }
    close(m_master_socket);
    for (auto &client : m_client_sockets) {
        close(client);
    }
}

void signal_handler(int signal)
{
    interrupted = true;
}

int main()
{
    signal(SIGINT, signal_handler);
    try {
        db::server::Server server{};
        server.work();
    } catch (const std::exception &err) {
        return 1;
    }
    return 0;
}
