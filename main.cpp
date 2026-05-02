#include "gamewidget.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 解决中文乱码
    qInstallMessageHandler(nullptr);
    a.installNativeEventFilter(nullptr);

    GameWidget w;
    w.show();
    return a.exec();
}
