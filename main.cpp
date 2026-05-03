#include "gamewidget.h"

#include <QApplication>
#include <QDebug>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置全局默认字体（支持中文的像素风格字体）
    QFont defaultFont("SimHei", 12);
    defaultFont.setStyleStrategy(QFont::NoAntialias);
    a.setFont(defaultFont);

    GameWidget w;
    w.show();
    return a.exec();
}