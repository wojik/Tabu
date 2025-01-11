#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>

class GameScreen : public QWidget {
    Q_OBJECT

public:
    explicit GameScreen(QWidget *parent = nullptr);

signals:
    void roundEnded();

private:
    QTextEdit *descriptionField;
    QLabel *roundNumberLabel;
    QPushButton *endRoundButton;
};

#endif // GAMESCREEN_H
