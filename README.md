# Taboo Game

Taboo Game to sieciowa gra słowna, w której gracze rywalizują, starając się odgadnąć słowo na podstawie podpowiedzi narrotora. Narrator opisuje hasło pozostałym graczom, lecz nie może używać słów z listy słów zakazanych dla danego hasła. 

## Zawartość repozytorium
- `serwer.cpp` - Kod źródłowy aplikacji serwera.
- `klient.cpp` - Kod źródłowy aplikacji klienta.
- `CMakeLists.txt` - Plik konfiguracji budowania projektu.
- `dane.txt` - przykładowy plik z danymi (hasłami) gry.

## Wymagania
- **System operacyjny:** Linux / Windows (z WSL).
- **Komponenty:**
  - Kompilator zgodny z C++17 (np. GCC lub Clang).
  - CMake w wersji 3.10 lub nowszej.

## Instrukcja uruchamiania
1. **Utwórz katalog `build` i skonfiguruj projekt**
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

2. **Skompiluj projekt**
   ```bash
   make
   ```
   Powstaną dwa pliki wykonywalne:
   - `server` - Serwer gry.
   - `client` - Klient gry.

3. **Uruchomienie serwera**
   
   W terminalu, w katalogu `build`:
   ```bash
   ./server
   ```

5. **Uruchomienie klienta**
   
   W innym terminalu, w katalogu `build`:
   ```bash
   ./client
   ```



---
**Autorzy:**
- Wojciech Kazimierski (156 004)
- Mateusz Krupa (155 879)
