#include "gamewidget.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GameWidget w;
    w.show();
    return a.exec();
}
