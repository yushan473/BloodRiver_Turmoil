#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include<QTimer>
#include<QMouseEvent>
#include<QList>
#include<QProgressBar>
#include<QLabel>
#include<QCursor>
#include<QApplication>
#include<QMediaPlayer>
#include<QAudioOutput>

#include "Player.h"
#include "Enemy.h"
#include "AnimationClip.h"

class Player;


class GameWidget : public QWidget
{
    Q_OBJECT

public:
    GameWidget(QWidget *parent = nullptr);
    ~GameWidget();
    //初始化
    void Init_Game();
    void startTimer();
    void stopTimer();
    //游戏状态
    bool isRunning = 0;//是否进行中
    bool isChoosing = 0;//是否选择技能
    bool isStopped = 0;//是否暂停
    bool isPressed = 0;//是否按住鼠标
    //输入事件
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event)override;
    //玩家移动
    void playerMove();
    //获取实例
    static GameWidget* widget;
private slots:
    void gameUpdate();
private:
    Player m_player;
    QList<Enemy> m_enemies;
    AnimationClip m_idleClip;
    AnimationClip m_walkClip;
    AnimationClip m_attackClip;

    // 辅助函数
    void loadAnimations();         // 创建测试动画（用纯色矩形）
    void checkAttackHit();         // 攻击命中检测

};
#endif // GAMEWIDGET_H
