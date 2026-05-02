#include "GameWidget.h"
#include "Player.h"
#include "Enemy.h"
#include "AnimationClip.h"
#include "AnimationPlayer.h"

#include <QPainter>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    // 原有的窗口设置
    setFixedSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);  // 确保键盘事件
    Init_Game();   // 你的原有函数
    startTimer();  // 启动定时器
    widget = this;

    loadAnimations();   // 加载动画资源
    m_player.setClips(&m_idleClip, &m_walkClip, &m_attackClip);

    // 创建几个敌人
    m_enemies.append(Enemy(QPointF(200, 300)));
    m_enemies.append(Enemy(QPointF(600, 400)));
}

void GameWidget::loadAnimations()
{
    // 待机动画：两帧，浅绿和深绿交替，每帧200ms
    QPixmap idle0(40,40); idle0.fill(Qt::lightGray);
    QPixmap idle1(40,40); idle1.fill(Qt::gray);
    m_idleClip.addFrame(idle0, 200);
    m_idleClip.addFrame(idle1, 200);

    // 走路动画：4帧，蓝色系
    QPixmap walk0(40,40); walk0.fill(Qt::cyan);
    QPixmap walk1(40,40); walk1.fill(Qt::darkCyan);
    QPixmap walk2(40,40); walk2.fill(Qt::cyan);
    QPixmap walk3(40,40); walk3.fill(Qt::darkCyan);
    m_walkClip.addFrame(walk0, 80);
    m_walkClip.addFrame(walk1, 80);
    m_walkClip.addFrame(walk2, 80);
    m_walkClip.addFrame(walk3, 80);

    // 攻击动画：红色闪烁，每帧50ms，共4帧
    QPixmap att0(40,40); att0.fill(Qt::red);
    QPixmap att1(40,40); att1.fill(Qt::darkRed);
    m_attackClip.addFrame(att0, 50);
    m_attackClip.addFrame(att1, 50);
    m_attackClip.addFrame(att0, 50);
    m_attackClip.addFrame(att1, 50);
}
void GameWidget::gameUpdate()
{
    static QElapsedTimer elapsed;
    if (!elapsed.isValid()) elapsed.start();
    float deltaSec = elapsed.restart() / 1000.0f;
    if (deltaSec > 0.033f) deltaSec = 0.033f;  // 限制最大帧间隔

    if (!isRunning || isStopped) return;  // 根据你的游戏状态控制

    // 1. 更新玩家
    m_player.update(deltaSec);

    // 2. 更新敌人（传入玩家X坐标用于AI）
    for (auto& enemy : m_enemies) {
        enemy.update(deltaSec, m_player.getCollisionRect().center().x());
    }

    // 3. 攻击命中检测（如果玩家攻击了，需要在玩家attack()中触发，或者每帧检测攻击帧）
    // 简单方式：在玩家attack()被调用时，我们立即检测一次碰撞，然后减少敌人生命。
    // 我们稍后会在按键处理中调用 m_player.attack()，然后调用 checkAttackHit()

    // 4. 移除死亡的敌人
    for (int i = m_enemies.size()-1; i >= 0; --i) {
        if (!m_enemies[i].isAlive()) {
            m_enemies.removeAt(i);
        }
    }

    // 5. 检查游戏结束（玩家死亡）
    if (!m_player.isAlive()) {
        isRunning = false;
        // 显示游戏结束文字等
    }

    // 6. 重绘
    update();  // 触发 paintEvent
}

void GameWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    // 绘制玩家
    m_player.draw(&painter);

    // 绘制所有敌人
    for (const auto& enemy : m_enemies) {
        enemy.draw(&painter);
    }

    // 绘制UI：玩家血条
    int barWidth = 200;
    int barHeight = 20;
    painter.setBrush(Qt::gray);
    painter.drawRect(10, 10, barWidth, barHeight);
    painter.setBrush(Qt::green);
    int healthPercent = m_player.getHealth() * barWidth / 100;
    painter.drawRect(10, 10, healthPercent, barHeight);
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (!isRunning) return;
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        m_player.setMoveLeft(true);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_player.setMoveRight(true);
        break;
    case Qt::Key_J:   // 攻击
        m_player.attack();
        // 攻击后立即检测命中（也可以延迟一帧，但为了简单，直接检测）
        checkAttackHit();
        break;
    case Qt::Key_P:   // 暂停
        isStopped = !isStopped;
        break;
    }
}

void GameWidget::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        m_player.setMoveLeft(false);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_player.setMoveRight(false);
        break;
    }
}

void GameWidget::checkAttackHit()
{
    // 获取玩家碰撞箱
    QRectF playerRect = m_player.getCollisionRect();
    for (int i = 0; i < m_enemies.size(); ++i) {
        if (playerRect.intersects(m_enemies[i].getCollisionRect())) {
            // 造成伤害（先使用第一个技能的伤害值）
            int dmg = 10;  // 可以改成从技能获取
            m_enemies[i].takeDamage(dmg);
            // 这里可以添加击退或特效
        }
    }
}

GameWidget::~GameWidget() {}
