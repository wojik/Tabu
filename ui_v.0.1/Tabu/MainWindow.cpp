#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    playerNameScreen = new PlayerNameScreen(this);
    lobbyScreen = new LobbyScreen(this);
    gameScreen = new GameScreen(this);
    resultsScreen = new ResultsScreen(this);

    setCentralWidget(playerNameScreen);

    connect(playerNameScreen, &PlayerNameScreen::nameConfirmed, [=](const QString &name) {
        emit playerNameConfirmed(name);
        setCentralWidget(lobbyScreen);
    });

    connect(lobbyScreen, &LobbyScreen::readyClicked, [=]() {
        emit playerReady();
        setCentralWidget(gameScreen);
    });

    connect(gameScreen, &GameScreen::roundEnded, [=]() {
        setCentralWidget(resultsScreen);
    });

    connect(resultsScreen, &ResultsScreen::backToMenu, [=]() {
        setCentralWidget(playerNameScreen);
    });
}
