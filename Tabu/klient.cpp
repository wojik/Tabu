#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <signal.h>
#include <algorithm>

#define PORT 12345
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

bool running = true;

void signal_handler(int signum) {
    running = false;
}

void receive_messages(int socket) {
    char buffer[BUFFER_SIZE];
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            std::cout << buffer;
        } else if (bytes_received == 0) {
            std::cout << "Rozłączono z serwerem.\n";
            running = false;
            break;
        }
    }
}

int main() {
    signal(SIGINT, signal_handler);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Błąd tworzenia socketu");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Nieprawidłowy adres");
        close(client_socket);
        return 1;
    }

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Połączenie nieudane");
        close(client_socket);
        return 1;
    }

    std::cout << "Połączono z serwerem\n";
    std::cout << "Podaj swój nick: ";
    std::string nickname;
    std::getline(std::cin, nickname);
    nickname += "\n";
    send(client_socket, nickname.c_str(), nickname.length(), 0);

    // Uruchom wątek odbierający wiadomości
    std::thread receive_thread(receive_messages, client_socket);

    std::string input;
    while (running) {
        std::getline(std::cin, input);
        input += "\n";
        
        if (input == "quit\n") {
            running = false;
        } else {
            send(client_socket, input.c_str(), input.length(), 0);
        }
    }

    close(client_socket);
    if (receive_thread.joinable()) {
        receive_thread.join();
    }
    return 0;
}