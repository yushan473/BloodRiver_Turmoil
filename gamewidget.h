#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include<QTimer>
#include<QMouseEvent>
#include<QPaintEvent>
#include<QList>
#include<QProgressBar>
#include<QLabel>
#include<QCursor>
#include<QApplication>
#include<QMediaPlayer>
#include<QAudioOutput>

#include "Player.h"
#include "Enemy.h"
#include "NPC.h"
#include "AnimationClip.h"

class Player;

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    GameWidget(QWidget *parent = nullptr);
    ~GameWidget();
    void Init_Game();
    void startTimer();
    void stopTimer();
    bool isRunning = 0;
    bool isChoosing = 0;
    bool isStopped = 0;
    bool isPressed = 0;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event)override;
    void paintEvent(QPaintEvent* event) override;
    void playerMove();
    static GameWidget* widget;

private slots:
    void gameUpdate();
private:
    Player m_player;
    QList<Enemy> m_enemies;
    NPC* m_npc = nullptr;
    QPixmap m_background;

    AnimationClip m_idleClip;
    AnimationClip m_walkClip;
    AnimationClip m_attackLv1Clip;
    AnimationClip m_attackLv2Clip;

    AnimationClip m_xieqianjiClip;
    AnimationClip m_muyinzhenClip;
    AnimationClip m_suzeClip;

    AnimationClip m_npcSuzhiClip;

    QTimer m_timer;

    enum GamePhase {
        Phase_Start,
        Phase_Help,
        Phase_Xieqianji,
        Phase_Suzhi_Dialog,
        Phase_Muyinzhen,
        Phase_Suze,
        Phase_Complete,
        Phase_GameOver
    };
    GamePhase m_currentPhase = Phase_Start;
    bool m_xieqianjiDefeated = false;
    bool m_muyinzhenDefeated = false;
    bool m_suzeDefeated = false;
    bool m_showDialog = false;
    bool m_isPlayerInvincible = false;
    qint64 m_lastDamageTime = 0;
    const int m_damageCooldownMs = 1000;

    const int m_defenseDamage = 5;
    const int m_attackDamage = 10;

    void loadAnimations();
    void loadBackground();
    void checkAttackHit(int damage);
    void handleGamePhase();
    void showGameComplete();
};

#endif // GAMEWIDGET_H