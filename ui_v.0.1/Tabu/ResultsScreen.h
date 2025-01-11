#ifndef RESULTSSCREEN_H
#define RESULTSSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class ResultsScreen : public QWidget {
    Q_OBJECT

public:
    explicit ResultsScreen(QWidget *parent = nullptr);

signals:
    void backToMenu();

private:
    QLabel *resultsLabel;
    QPushButton *backToMenuButton;
};

#endif // RESULTSSCREEN_H
