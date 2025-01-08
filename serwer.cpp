
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define PORT 12345
#define MAX_EVENTS 10
#define MAX_CLIENTS 100

void set_nonblocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }

    set_nonblocking(server_socket);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Epoll creation failed");
        close(server_socket);
        return 1;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = server_socket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
        perror("Epoll control failed");
        close(server_socket);
        close(epoll_fd);
        return 1;
    }

    std::vector<int> clients;
    epoll_event events[MAX_EVENTS];

    std::cout << "Server is running on port " << PORT << "...\n";

    while (true) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("Epoll wait failed");
            break;
        }

        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == server_socket) {
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

                if (client_socket == -1) {
                    perror("Accept failed");
                    continue;
                }

                set_nonblocking(client_socket);
                
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_socket;
                
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    perror("Epoll add client failed");
                    close(client_socket);
                    continue;
                }

                clients.push_back(client_socket);
                std::cout << "New client connected: " << client_socket << "\n";
            } else {
                // Placeholder for further communication logic
                std::cout << "Data from client: " << events[i].data.fd << "\n";
            }
        }
    }

    for (int client_socket : clients) {
        close(client_socket);
    }

    close(server_socket);
    close(epoll_fd);
    return 0;
}
