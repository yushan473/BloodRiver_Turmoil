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
#include "fragment.h"

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
    QList<Enemy*> enemies;
    NPC* npc = nullptr;

    QPixmap background;
    int cameraX = 0;
    //const int bgwidth = 512;
    const int window_wid = 512;
    const int player_pos = 200;

    QTimer timer;
    // 玩家动画
    AnimationClip idleClip;
    AnimationClip walkClip;
    AnimationClip attackLv1Clip;
    AnimationClip attackLv2Clip;
    AnimationClip jumpClip;
    AnimationClip superClip;
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
        PHASE_INGAME,
        PHASE_SUZHI_DIALOG,
        PHASE_WIN,
        PHASE_GAMEOVER
    };
    int gameState = PHASE_START;

    // 计时 & NPC 冻结
    float gameTimer = 0.0f;
    bool isFrozen = false;
    int freezeNpcIndex = 0;
    const float freezeTimes[3] = {8.0f};

    bool showDialog = false;
    int currentDialogIndex = 0;

    bool isPlayerInvincible = false;

    QPixmap startImage;
    QPixmap helpImage;
    QPixmap endImage;
    QPixmap successImage;
    QList<QPixmap> dialogImages;

    const int defenseDamage = 10;
    const int attackDamage = 20;

    QMediaPlayer* bgMusic = nullptr;
    QAudioOutput* audioOutput = nullptr;
    bool musicEnabled = true;

    QPixmap musicOnImage;
    QPixmap musicOffImage;

    void loadAnimations();
    void loadBackground();
    void checkHit(int damage);
    void resetToStart();

    //enemy生成
    float spawnTimer = 0.0f;
    float spawnInterval = 3.0f;
    const float MAX_SPAWN_INTERVAL = 3.0f;
    const float MIN_SPAWN_INTERVAL = 1.0f;
    const int MAX_ENEMIES = 3;

    //fragment
    QList<Fragment*> fragments;
    int collectedFragments = 0;
    const int TARGET_FRAGMENTS = 20;
    float distanceSinceLastFragment = 0.0f;
    const float FRAGMENT_SPAWN_DISTANCE = 300.0f;

};

#endif // GAMEWIDGET_H
