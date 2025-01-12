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
#include <fstream>
#include <sstream>
#include <cctype>

#define PORT 12345
#define MAX_EVENTS 10
#define MAX_CLIENTS 100

struct Player {
    int socket;
    std::string nickname;
    int points;
    bool ready;
    bool is_narrator;
};

struct TabooWord {
    std::string word;
    std::vector<std::string> forbidden_words;
};

class TabooGame {
private:
    std::vector<Player> players;
    std::vector<Player> waiting_players;
    std::vector<TabooWord> words;
    int round_count = 0;
    bool round_in_progress = false;
    TabooWord current_word;
    int narrator_socket = -1;

    void load_words(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;
        
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string word, forbidden;
            std::vector<std::string> forbidden_words;
            
            std::getline(iss, word, ':');
            while (std::getline(iss, forbidden, ',')) {
                // Usuń białe znaki
                forbidden.erase(0, forbidden.find_first_not_of(" "));
                forbidden.erase(forbidden.find_last_not_of(" ") + 1);
                forbidden_words.push_back(forbidden);
            }
            
            words.push_back({word, forbidden_words});
        }
    }

    void send_message(int socket, const std::string& message) {
        send(socket, message.c_str(), message.size(), 0);
    }

    void broadcast_message(const std::string& message, int exclude_socket = -1) {
        for (const auto& player : players) {
            if (player.socket != exclude_socket) {
                send_message(player.socket, message);
            }
        }
    }

    void broadcast_player_list() {
        std::string player_list = "\nLista graczy:\n";
        for (const auto& player : players) {
            player_list += player.nickname + (player.is_narrator ? " (NARRATOR)" : "") + 
                          ": " + std::to_string(player.points) + " punktów\n";
        }
        broadcast_message(player_list);
    }

    bool is_nickname_taken(const std::string& nickname) {
        for (const auto& player : players) {
            if (player.nickname == nickname) return true;
        }
        for (const auto& player : waiting_players) {
            if (player.nickname == nickname) return true;
        }
        return false;
    }
	
    void start_round() {
        if (players.size() < 3) return;

        round_in_progress = true;
        current_word = words[rand() % words.size()];

        // Wyślij informacje do narratora
        std::string narrator_msg = "\nJesteś narratorem! Twoje hasło to: " + current_word.word + "\n";
        narrator_msg += "Słowa zabronione: ";
        for (const auto& word : current_word.forbidden_words) {
            narrator_msg += word + ", ";
        }
        narrator_msg += "\n";
        send_message(narrator_socket, narrator_msg);

        // Poinformuj pozostałych graczy
        broadcast_message("\nNowa runda rozpoczęta! Czekaj na podpowiedzi narratora.\n");
    }

	void end_round() {
        round_in_progress = false;
        broadcast_message("\nKoniec rundy! Hasło to było: " + current_word.word + "\n");
        

        round_count++;
        if (round_count >= 5) {
            end_game();
        } else {
            // Dodaj oczekujących graczy
            for (auto& waiting_player : waiting_players) {
                players.push_back(waiting_player);
                broadcast_message("Gracz " + waiting_player.nickname + " dołączył do gry!\n");
            }
            waiting_players.clear();
            broadcast_player_list();
            // Reset gotowości graczy
            for (auto& player : players) {
                if (!player.is_narrator) {
                    player.ready = false;
                    send_message(player.socket, "\nCzy jesteś gotów na następną rundę? (wpisz 'ready')\n");
                }
            }
        }
    }

    void end_game() {
        std::string final_ranking = "Koniec gry! Końcowy ranking:\n";
        std::sort(players.begin(), players.end(), 
            [](const Player& a, const Player& b) { return a.points > b.points; });
            
        for (const auto& player : players) {
            final_ranking += player.nickname + ": " + std::to_string(player.points) + " punktów\n";
        }
        broadcast_message(final_ranking);
        
        // Reset gry
        round_count = 0;
        for (auto& player : players) {
            player.points = 0;
            player.ready = false;
            send_message(player.socket, "Nowa gra rozpocznie się gdy wszyscy będą gotowi (wpisz 'ready')\n");
        }
    }
	
	void select_random_narrator() {
        if (players.empty()) return;
        int narrator_index = rand() % players.size();
        players[narrator_index].is_narrator = true;
        narrator_socket = players[narrator_index].socket;
        send_message(narrator_socket, "\nZostałeś wybrany narratorem na tę grę!\n");
    }

public:
    TabooGame(const std::string& words_file) {
        load_words(words_file);
        srand(time(nullptr));
    }

    bool add_player(int socket, const std::string& nickname) {

        if (is_nickname_taken(nickname)) {
        send_message(socket, "Ten pseudonim jest już zajęty. Podaj inny:\n");
        return false;
		}

		Player new_player = {socket, nickname, 0, false, false};
        
		if (round_in_progress) {
            waiting_players.push_back(new_player);
            send_message(socket, "\nGra w toku. Zostaniesz dodany w następnej rundzie.\n");
        } else {
            players.push_back(new_player);
            broadcast_message("\nGracz " + nickname + " dołączył do gry!\n");
            if (!new_player.is_narrator) {
                send_message(socket, "\nWpisz 'ready' gdy będziesz gotowy do gry.\n");
            }
        }
        broadcast_player_list();
		return true;

    }
	
	void remove_player(int socket) {
        auto it = std::find_if(players.begin(), players.end(),
            [socket](const Player& p) { return p.socket == socket; });
        
        if (it != players.end()) {
            broadcast_message("\nGracz " + it->nickname + " opuścił grę.\n", socket);
            // Jeśli narrator opuścił grę, kończymy grę
            if (it->is_narrator) {
                broadcast_message("\nNarrator opuścił grę. Gra zostanie zakończona.\n");
                players.clear();
                round_in_progress = false;
                round_count = 0;
                return;
            }
            players.erase(it);
            broadcast_player_list();
        }
    }

    void handle_message(int socket, std::string message) {
        // Usuń znak nowej linii
        message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());
        
        auto player_it = std::find_if(players.begin(), players.end(),
            [socket](const Player& p) { return p.socket == socket; });
            
        if (player_it == players.end()) return;

		if (message == "exit") {
			send_message(socket, "Opuszczasz grę. Do widzenia!\n");
			remove_player(socket);
			close(socket);
			return;
		}

        if (!round_in_progress) {
            if (message == "ready") {
                player_it->ready = true;
                broadcast_message(player_it->nickname + " jest gotowy!\n");
                
                // Sprawdź czy wszyscy są gotowi
                bool all_ready = std::all_of(players.begin(), players.end(),
                    [](const Player& p) { return p.ready; });
                    
                if (all_ready && players.size() >= 3) {
                    if (narrator_socket == -1) {
                        select_random_narrator();
                    }
                    start_round();
                }
            }
        } else {
            // Pokaż wszystkie wiadomości wszystkim graczom
            std::string prefix = player_it->is_narrator ? "[NARRATOR] " : "";
            broadcast_message(prefix + player_it->nickname + ": " + message + "\n");

            if (!player_it->is_narrator) {
                // Sprawdź czy odpowiedź jest poprawna
                std::string lower_guess = message;
                std::transform(lower_guess.begin(), lower_guess.end(), lower_guess.begin(), ::tolower);
                
                std::string lower_word = current_word.word;
                std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);

                if (lower_guess == lower_word) {
                    player_it->points += 2;  // Punkty dla odgadującego
                    // Znajdź narratora i dodaj mu punkt
                    auto narrator_it = std::find_if(players.begin(), players.end(),
                        [](const Player& p) { return p.is_narrator; });
                    if (narrator_it != players.end()) {
                        narrator_it->points += 1;
                    }
                    broadcast_message("\n" + player_it->nickname + " odgadł hasło!\n");
                    end_round();
                }
            } else {
                // Sprawdź czy narrator nie użył zabronionych słów
                std::string lower_msg = message;
                std::transform(lower_msg.begin(), lower_msg.end(), lower_msg.begin(), ::tolower);
                
                for (const auto& forbidden : current_word.forbidden_words) {
                    std::string lower_forbidden = forbidden;
                    std::transform(lower_forbidden.begin(), lower_forbidden.end(), 
                                 lower_forbidden.begin(), ::tolower);
                    if (lower_msg.find(lower_forbidden) != std::string::npos) {
                        send_message(socket, "\nUżyłeś zabronionego słowa! Minus 1 punkt.\n");
                        player_it->points--;
                        broadcast_player_list();
                        break;
                    }
                }
            }
        }
    }
};

int main() {
    TabooGame game("dane.txt");
    
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

    std::map<int, bool> nickname_set;  // Śledzi, czy gracz już podał nick
    std::map<int, std::string> buffered_data;  // Bufor na niekompletne dane
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
                // Nowe połączenie
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

                if (client_socket == -1) {
                    perror("Accept failed");
                    continue;
                }

                // Ustaw socket jako nieblokujący
                fcntl(client_socket, F_SETFL, O_NONBLOCK);

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    perror("Epoll add client failed");
                    close(client_socket);
                    continue;
                }

                nickname_set[client_socket] = false;
                send(client_socket, "Podaj swój nick: ", 18, 0);

            } else {
                // Dane od istniejącego klienta
                char buffer[1024];
                int client_socket = events[i].data.fd;
                int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    buffered_data[client_socket] += buffer;

                    // Sprawdź czy mamy kompletną linię
                    size_t pos;
                    while ((pos = buffered_data[client_socket].find('\n')) != std::string::npos) {
                        std::string line = buffered_data[client_socket].substr(0, pos);
                        buffered_data[client_socket] = buffered_data[client_socket].substr(pos + 1);

                        if (!nickname_set[client_socket]) {
                            // Obsługa nowego gracza
                            if (line.empty()) continue;
                            

                            	nickname_set[client_socket] = game.add_player(client_socket, line);
                        } else {
                            // Obsługa wiadomości od istniejącego gracza
                            game.handle_message(client_socket, line);
                        }
                    }
                } else if (bytes_received == 0 || (bytes_received == -1 && errno != EAGAIN)) {
                    // Klient się rozłączył
                    game.remove_player(client_socket);
                    nickname_set.erase(client_socket);
                    buffered_data.erase(client_socket);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
                    close(client_socket);
                }
            }
        }
    }

    close(server_socket);
    close(epoll_fd);
    return 0;
}