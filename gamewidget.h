#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    GameWidget(QWidget *parent = nullptr);
    ~GameWidget();
};
#endif // GAMEWIDGET_H
