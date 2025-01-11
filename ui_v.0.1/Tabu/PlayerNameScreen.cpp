#include "PlayerNameScreen.h"
#include <QVBoxLayout>

PlayerNameScreen::PlayerNameScreen(QWidget *parent) : QWidget(parent) {
    nameInput = new QLineEdit(this);
    confirmButton = new QPushButton("Confirm", this);
    errorMessage = new QLabel(this);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Enter Player Name:", this));
    layout->addWidget(nameInput);
    layout->addWidget(confirmButton);
    layout->addWidget(errorMessage);

    connect(confirmButton, &QPushButton::clicked, this, &PlayerNameScreen::confirmName);
}

void PlayerNameScreen::confirmName() {
    const QString name = nameInput->text().trimmed();
    if (name.isEmpty()) {
        errorMessage->setText("Name cannot be empty.");
        return;
    }
    // Simulate name validation here.
    bool nameIsValid = true; // Replace with real validation.
    if (!nameIsValid) {
        errorMessage->setText("Name already taken. Choose another.");
        return;
    }
    emit nameConfirmed(name);
}
