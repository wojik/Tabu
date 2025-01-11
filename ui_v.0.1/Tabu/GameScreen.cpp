#include "GameScreen.h"
#include "QVBoxLayout"

GameScreen::GameScreen(QWidget *parent) : QWidget(parent) {
    descriptionField = new QTextEdit(this);
    roundNumberLabel = new QLabel("Round: 1", this);
    endRoundButton = new QPushButton("End Round", this);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(roundNumberLabel);
    layout->addWidget(new QLabel("Describe the word without using banned words:", this));
    layout->addWidget(descriptionField);
    layout->addWidget(endRoundButton);

    connect(endRoundButton, &QPushButton::clicked, this, &GameScreen::roundEnded);
}
