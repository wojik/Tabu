#ifndef PLAYERNAMESCREEN_H
#define PLAYERNAMESCREEN_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class PlayerNameScreen : public QWidget {
    Q_OBJECT

public:
    explicit PlayerNameScreen(QWidget *parent = nullptr);

signals:
    void nameConfirmed(const QString &name);

private slots:
    void confirmName();

private:
    QLineEdit *nameInput;
    QPushButton *confirmButton;
    QLabel *errorMessage;
};

#endif // PLAYERNAMESCREEN_H
