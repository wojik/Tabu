#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "PlayerNameScreen.h"
#include "LobbyScreen.h"
#include "GameScreen.h"
#include "ResultsScreen.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:
    void playerNameConfirmed(const QString &name);
    void playerReady();

private:
    PlayerNameScreen *playerNameScreen;
    LobbyScreen *lobbyScreen;
    GameScreen *gameScreen;
    ResultsScreen *resultsScreen;
};

#endif // MAINWINDOW_H

