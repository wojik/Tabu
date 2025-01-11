#include "ResultsScreen.h"
#include "QVBoxLayout"

ResultsScreen::ResultsScreen(QWidget *parent) : QWidget(parent) {
    resultsLabel = new QLabel("Results of the round", this);
    backToMenuButton = new QPushButton("Back to Menu", this);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(resultsLabel);
    layout->addWidget(backToMenuButton);

    connect(backToMenuButton, &QPushButton::clicked, this, &ResultsScreen::backToMenu);
}
