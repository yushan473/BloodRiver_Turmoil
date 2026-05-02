QT       += core gui widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    animationclip.cpp \
    animationplayer.cpp \
    enemy.cpp \
    entity.cpp \
    gamewidget.cpp \
    main.cpp \
    player.cpp \
    skill.cpp

HEADERS += \
    animationclip.h \
    animationplayer.h \
    enemy.h \
    entity.h \
    gamewidget.h \
    player.h \
    skill.h \
    transform.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
