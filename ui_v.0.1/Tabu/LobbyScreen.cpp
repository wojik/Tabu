#include "LobbyScreen.h"
#include <QVBoxLayout>
#include <QLabel>

LobbyScreen::LobbyScreen(QWidget *parent) : QWidget(parent) {
    playerList = new QListWidget(this);
    readyCheckBox = new QCheckBox("Ready", this);
    readyButton = new QPushButton("Confirm Ready", this);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Players in Lobby:", this));
    layout->addWidget(playerList);
    layout->addWidget(readyCheckBox);
    layout->addWidget(readyButton);

    connect(readyButton, &QPushButton::clicked, [=]() {
        if (readyCheckBox->isChecked()) {
            emit readyClicked();
        }
    });
}
