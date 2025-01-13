cmake_minimum_required(VERSION 3.10)
project(TabooGame)

# Ustawienia kompilatora
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Pliki źródłowe
set(SERVER_SOURCE serwer.cpp)
set(CLIENT_SOURCE klient.cpp)

# Tworzenie aplikacji serwera
add_executable(serwer ${SERVER_SOURCE})

# Tworzenie aplikacji klienta
add_executable(klient ${CLIENT_SOURCE})

# Ustawienia instalacji (opcjonalnie)
install(TARGETS serwer klient
        RUNTIME DESTINATION bin)

# Dodatkowe informacje dla użytkownika
message(STATUS "Projekt TabooGame został skonfigurowany. Użyj 'make' aby skompilować.")
