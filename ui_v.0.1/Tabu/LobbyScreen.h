#ifndef LOBBYSCREEN_H
#define LOBBYSCREEN_H

#include <QWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>

class LobbyScreen : public QWidget {
    Q_OBJECT

public:
    explicit LobbyScreen(QWidget *parent = nullptr);

signals:
    void readyClicked();

private:
    QListWidget *playerList;
    QCheckBox *readyCheckBox;
    QPushButton *readyButton;
};

#endif // LOBBYSCREEN_H
