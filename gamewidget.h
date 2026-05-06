#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QTimer>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QList>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "Player.h"
#include "Enemy.h"
#include "NPC.h"
#include "AnimationClip.h"

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    GameWidget(QWidget *parent = nullptr);
    ~GameWidget();
    
    void Init_Game();
    void startTimer(); //启动计时
    void stopTimer(); //停止计时
    
    // 游戏状态
    bool isRunning = 0;
    bool isChoosing = 0;
    bool isStopped = 0;
    bool isPressed = 0;

    void keyPressEvent(QKeyEvent* event) override;// 键盘按下
    void keyReleaseEvent(QKeyEvent* event) override;// 键盘释放
    void paintEvent(QPaintEvent* event) override;// 绘制
    void mousePressEvent(QMouseEvent *event) override;// 鼠标点击

private slots:
    void gameUpdate();
    void onInvincibilityEnd();
private:
    Player player;
    QList<Enemy> enemies;
    NPC* npc = nullptr;
    QPixmap background;
    QTimer timer;
    // 玩家动画
    AnimationClip idleClip;
    AnimationClip walkClip;
    AnimationClip attackLv1Clip;
    AnimationClip attackLv2Clip;
    // 敌人动画
    AnimationClip xieqianjiClip;
    AnimationClip muyinzhenClip;
    AnimationClip suzeClip;
    // NPC动画
    AnimationClip npcSuzhiClip;
    // 游戏状态
    enum {
        PHASE_START,
        PHASE_HELP,
        PHASE_XIEQIANJI,
        PHASE_SUZHI_DIALOG,
        PHASE_MUYINZHEN,
        PHASE_SUZE,
        PHASE_WIN,
        PHASE_GAMEOVER
    };
    int gameState = PHASE_START;

    bool xieqianjiDefeated = false;
    bool muyinzhenDefeated = false;
    bool suzeDefeated = false;

    bool showDialog = false;
    int currentDialogIndex = 0;

    bool isPlayerInvincible = false;

    QPixmap startImage;
    QPixmap helpImage;
    QPixmap endImage;
    QPixmap successImage;
    QList<QPixmap> dialogImages;

    const int defenseDamage = 5;
    const int attackDamage = 10;

    QMediaPlayer* bgMusic = nullptr;
    QAudioOutput* audioOutput = nullptr;
    bool musicEnabled = true;

    QPixmap musicOnImage;
    QPixmap musicOffImage;

    void loadAnimations();
    void loadBackground();
    void checkHit(int damage);
    void resetToStart();
};

#endif // GAMEWIDGET_H
