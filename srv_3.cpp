#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <algorithm>
#include <ctime>
#include <thread>
#include <chrono>

#define PORT 12345
#define MAX_EVENTS 10
#define MAX_CLIENTS 100

void set_nonblocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

bool contains_forbidden_word(const std::string& message, const std::vector<std::string>& forbidden_words) {
    for (const auto& word : forbidden_words) {
        if (message.find(word) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void send_to_all(const std::vector<int>& clients, const std::string& message) {
    for (int client : clients) {
        send(client, message.c_str(), message.size(), 0);
    }
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
    std::map<int, bool> client_ready;
    std::map<int, std::string> player_nicks;
    std::map<int, int> player_points;
    epoll_event events[MAX_EVENTS];

    // Lista haseł i słów zabronionych
    std::map<std::string, std::vector<std::string>> taboo_words = {
        {"mleko", {"mleko", "biały"}},
        {"bekon", {"bekon", "świnia"}}
    };

    int taboo_player = -1;
    std::string current_taboo;
    int taboo_violations = 0;
    std::vector<int> guess_order;

    int round_count = 0;
    bool round_in_progress = false;
    std::vector<int> new_clients;

    std::cout << "Server is running on port " << PORT << "...\n";

    srand(time(nullptr));

    auto start_timer = [&]() {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        send_to_all(clients, "Czas na odgadnięcie hasła minął!\n");

        // Wysyłanie aktualnego rankingu
        std::string ranking = "Aktualny ranking:\n";
        for (const auto& [client, points] : player_points) {
            ranking += "Gracz " + std::to_string(client) + ": " + std::to_string(points) + " punktów\n";
        }
        send_to_all(clients, ranking);

        round_in_progress = false;
        round_count++;

        if (round_count >= 5) {
            // Zakończenie gry
            std::string end_message = "Gra zakończona! Oto ostateczny ranking:\n" + ranking;
            send_to_all(clients, end_message);

            for (int client : clients) {
                const char* ready_message = "Are you ready?\n";
                send(client, ready_message, strlen(ready_message), 0);
                client_ready[client] = false;
            }

            round_count = 0;
            player_points.clear();
            taboo_player = -1;
            current_taboo.clear();
            new_clients.clear();
        } else {
            // Rozpoczęcie nowej tury
            if (!new_clients.empty()) {
                for (int client : new_clients) {
                    const char* wait_message = "Trwa tura, zostaniesz dodany do kolejnej rundy.\n";
                    send(client, wait_message, strlen(wait_message), 0);
                    clients.push_back(client);
                    client_ready[client] = false;
                    player_points[client] = 10;
                }
                new_clients.clear();
            }

            taboo_player = -1;
            current_taboo.clear();
        }
    };

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

                if (round_in_progress) {
                    new_clients.push_back(client_socket);
                    const char* wait_message = "Trwa tura, zostaniesz dodany do kolejnej rundy.\n";
                    send(client_socket, wait_message, strlen(wait_message), 0);
                } else {
                    clients.push_back(client_socket);
                    client_ready[client_socket] = false;
                    player_points[client_socket] = 0;
                    player_nicks[client_socket] = "";
                }

                std::cout << "New client connected: " << player_nicks[client_socket] << "\n";
                const char* message = "Give us your nickname.\n";
                send(client_socket, message, strlen(message), 0);

            } else {
                char buffer[1024];
                int client_socket = events[i].data.fd;
                int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    std::string message(buffer);

                    if(player_nicks[client_socket] == ""){

                        bool foundNick = false;
                        for (const auto& placeHolder : player_nicks)
                        {
                            if (placeHolder.second == message)
                            {
                                foundNick = true;
                                break;
                            
                            }
                            
                        }
                        
                        if ( foundNick )
                        {
                            
                        const char* messageAgain = "Given nick is already taken, give us something more creative.\n";
                        send(client_socket, messageAgain, strlen(messageAgain), 0);

                        }else{
                         
                        player_nicks[client_socket] = message;
                        const char* messageReady = "Are you ready.\n";
                        send(client_socket, messageReady, strlen(messageReady), 0);

                        }

                    }else if (!round_in_progress && message == "ready\n") {
                        client_ready[client_socket] = true;
                        std::cout << "Client " << player_nicks[client_socket] << " is ready.\n";

                        int ready_count = 0;
                        for (const auto& [socket, ready] : client_ready) {
                            if (ready) {
                                ready_count++;
                            }
                        }

                        if (ready_count == clients.size() && ready_count >= 3) {
                            std::cout << "All players are ready. Starting the game!\n";
                            round_in_progress = true;

                            // Wybór losowego gracza i hasła
                            taboo_player = clients[rand() % clients.size()];
                            auto it = taboo_words.begin();
                            std::advance(it, rand() % taboo_words.size());
                            current_taboo = it->first;

                            std::string player_message = "Znasz nasze taboo, spróbuj przekazać je innym. Hasło to '" + current_taboo + "'\n";
                            send(taboo_player, player_message.c_str(), player_message.size(), 0);

                            for (int client : clients) {
                                if (client != taboo_player) {
                                    const char* guess_message = "Spróbuj odgadnąć nasze taboo!\n";
                                    send(client, guess_message, strlen(guess_message), 0);
                                }
                            }
                            guess_order.clear();

                            // Uruchomienie timera
                            std::thread(start_timer).detach();
                        }
                    } else if (client_socket == taboo_player) {
                        if (contains_forbidden_word(message, taboo_words[current_taboo])) {
                            taboo_violations++;
                            std::string warning_message = "Użyłeś zakazanego słowa!\n";
                            send(taboo_player, warning_message.c_str(), warning_message.size(), 0);

                            if (taboo_violations >= 2) {
                                player_points[taboo_player] -= 1;
                                taboo_violations = 0;
                                std::string penalty_message = "Straciłeś 1 punkt!\n";
                                send(taboo_player, penalty_message.c_str(), penalty_message.size(), 0);
                            }
                        } else {
                            for (int client : clients) {
                                if (client != taboo_player) {
                                    send(client, message.c_str(), message.size(), 0);
                                }
                            }
                        }
                    } else if (message == current_taboo + "\n") {
                        if (std::find(guess_order.begin(), guess_order.end(), client_socket) == guess_order.end()) {
                            guess_order.push_back(client_socket);
                            int points = 1;  // Pierwszy gracz dostaje 1 punkt
                            player_points[client_socket] += points;

                            // Powiadomienie o poprawnym zgadnięciu
                            std::string success_message = "Zgadłeś hasło! Otrzymujesz 1 punkt!\n";
                            send(client_socket, success_message.c_str(), success_message.size(), 0);

                            // Powiadomienie o zgadnięciu hasła
                            std::string broadcast_message = "Gracz " + player_nicks[client_socket] + " odgadł hasło!\n";
                            for (int client : clients) {
                                if (client != client_socket) {
                                    send(client, broadcast_message.c_str(), broadcast_message.size(), 0);
                                }
                            }

                            // Koniec rundy, czekaj 5 sekund przed rozpoczęciem nowej
                            send_to_all(clients, "Runda zakończona! Zaczynamy odliczanie 5 sekund do nowej tury.\n");
                            
                            std::this_thread::sleep_for(std::chrono::seconds(5));  // Odliczanie 5 sekund

                            // Rozpoczęcie nowej tury
                            round_in_progress = false;
                            taboo_player = clients[rand() % clients.size()];  // Wybór nowego gracza znającego hasło
                            auto it = taboo_words.begin();
                            std::advance(it, rand() % taboo_words.size());
                            current_taboo = it->first;

                            // Powiadomienie dla nowego gracza tabu
                            std::string player_message = "Znasz nasze taboo, spróbuj przekazać je innym. Hasło to '" + current_taboo + "'\n";
                            send(taboo_player, player_message.c_str(), player_message.size(), 0);

                            // Powiadomienie dla innych graczy
                            for (int client : clients) {
                                if (client != taboo_player) {
                                    const char* guess_message = "Spróbuj odgadnąć nasze taboo!\n";
                                    send(client, guess_message, strlen(guess_message), 0);
                                }
                            }
                            guess_order.clear();  // Resetowanie kolejności zgadywania

                            // Uruchomienie timera dla nowej tury
                            std::thread(start_timer).detach();
                        }
                    }

                } else if (bytes_received == 0) {
                    std::cout << "Client " << player_nicks[client_socket] << " disconnected.\n";
                    close(client_socket);
                    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
                    client_ready.erase(client_socket);
                    player_points.erase(client_socket);
                    if (taboo_player == client_socket) {
                        taboo_player = -1;
                        current_taboo = "";
                        taboo_violations = 0;
                    }
                }
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
