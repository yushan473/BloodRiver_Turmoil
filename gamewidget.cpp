#include "GameWidget.h"
#include "Player.h"
#include "Enemy.h"
#include "NPC.h"
#include "AnimationClip.h"
#include "AnimationPlayer.h"

#include <QPainter>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QDateTime>
#include <QDebug>
#include <QRandomGenerator>
#include <QFont>

GameWidget* GameWidget::widget = nullptr;

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(512, 512);
    setFocusPolicy(Qt::StrongFocus);
    Init_Game();
    startTimer();
    widget = this;

    loadBackground();
    loadAnimations();

    m_player.setClips(&m_idleClip, &m_walkClip, &m_attackLv1Clip, &m_attackLv2Clip);

    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_PaintOnScreen, false);
}

GameWidget::~GameWidget()
{
    delete m_npc;
}

void GameWidget::loadBackground()
{
    m_background.load(":/res/image/background.png");
}

void GameWidget::loadAnimations()
{
    m_idleClip.loadFromSpriteSheet(":/res/image/p_idle.png", 3, 150);
    m_walkClip.loadFromSpriteSheet(":/res/image/player_walk01.png", 8, 100);
    m_attackLv1Clip.loadFromSpriteSheet(":/res/image/player_attack_lv1.png", 14, 70);
    m_attackLv2Clip.loadFromSpriteSheet(":/res/image/player_attack_lv2.png", 13, 77);

    m_xieqianjiClip.loadFromSpriteSheet(":/res/image/enemy_Xieqianji_attack.png", 5, 100);
    m_muyinzhenClip.loadFromSpriteSheet(":/res/image/enemy_Muyinzhen_attack.png", 5, 100);
    m_suzeClip.loadFromSpriteSheet(":/res/image/enemy_Suze_attack.png", 5, 100);

    m_npcSuzhiClip.loadFromSpriteSheet(":/res/image/npc_Suzhi_idle.png", 3, 100);
}

void GameWidget::gameUpdate()
{
    static QElapsedTimer elapsed;
    if (!elapsed.isValid()) elapsed.start();
    float deltaSec = elapsed.restart() / 1000.0f;
    if (deltaSec > 0.033f) deltaSec = 0.033f;

    if (!isRunning || isStopped) return;
    if (m_currentPhase == Phase_Start || m_currentPhase == Phase_Help) return;

    m_player.update(deltaSec);

    for (auto& enemy : m_enemies) {
        enemy.update(deltaSec, m_player.getCollisionRect().center());
    }

    if (m_npc) {
        m_npc->update(deltaSec);
        if (!m_showDialog && m_npc->isPlayerNearby(m_player.getCollisionRect().center())) {
            m_showDialog = true;
            m_npc->startDialog();
            isStopped = true;
        }
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    for (const auto& enemy : m_enemies) {
        if (enemy.getCollisionRect().intersects(m_player.getCollisionRect())) {
            if (!m_isPlayerInvincible && (currentTime - m_lastDamageTime) > m_damageCooldownMs) {
                m_player.takeDamage(5);
                m_lastDamageTime = currentTime;
                m_isPlayerInvincible = true;
                QTimer::singleShot(500, this, [this]() { m_isPlayerInvincible = false; });
            }
            break;
        }
    }

    if (!m_player.isAlive()) {
        m_currentPhase = Phase_GameOver;
        isStopped = true;
        return;
    }

    for (int i = m_enemies.size()-1; i >= 0; --i) {
        if (!m_enemies[i].isAlive()) {
            Enemy::EnemyType type = m_enemies[i].getType();
            m_enemies.removeAt(i);

            if (type == Enemy::Xieqianji && !m_xieqianjiDefeated) {
                m_xieqianjiDefeated = true;
                m_currentPhase = Phase_Suzhi_Dialog;
                m_npc = new NPC(QPointF(180, 192), NPC::Suzhi);
                m_npc->setIdleClip(&m_npcSuzhiClip);
            } else if (type == Enemy::Muyinzhen && !m_muyinzhenDefeated) {
                m_muyinzhenDefeated = true;
                m_currentPhase = Phase_Suze;
                Enemy suze(QPointF(200, 192), Enemy::Suze);
                suze.setAttackClip(&m_suzeClip);
                m_enemies.append(suze);
            } else if (type == Enemy::Suze && !m_suzeDefeated) {
                m_suzeDefeated = true;
                m_currentPhase = Phase_Complete;
                isRunning = false;
            }
        }
    }

    if (!m_player.isAlive()) {
        isRunning = false;
    }

    update();
}

void GameWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPixmap buffer(256, 256);
    QPainter bufferPainter(&buffer);

    if (!m_background.isNull()) {
        bufferPainter.drawPixmap(0, 0, m_background);
    } else {
        bufferPainter.fillRect(buffer.rect(), QColor(40, 40, 40));
    }

    bufferPainter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    QFont font;
    font.setFamily("SimHei");
    font.setStyleStrategy(QFont::NoAntialias);
    bufferPainter.setPen(Qt::white);

    if (m_currentPhase == Phase_Start) {
        font.setPointSize(16);
        bufferPainter.setFont(font);
        bufferPainter.drawText(buffer.rect(), Qt::AlignCenter, "暗河传");

        font.setPointSize(8);
        bufferPainter.setFont(font);
        int btnY = 120;
        bufferPainter.setBrush(QColor(60, 60, 80, 200));
        bufferPainter.drawRect(78, btnY, 100, 24);
        bufferPainter.drawRect(78, btnY + 34, 100, 24);
        bufferPainter.drawText(78, btnY, 100, 24, Qt::AlignCenter, "开始游戏");
        bufferPainter.drawText(78, btnY + 34, 100, 24, Qt::AlignCenter, "帮助说明");
        bufferPainter.drawText(buffer.rect(), Qt::AlignBottom | Qt::AlignHCenter, "Enter-开始 H-帮助");
    } else if (m_currentPhase == Phase_Help) {
        font.setPointSize(16);
        bufferPainter.setFont(font);
        bufferPainter.drawText(QRect(0, 20, 256, 30), Qt::AlignCenter, "帮助说明");

        font.setPointSize(8);
        bufferPainter.setFont(font);
        QStringList helpLines;
        helpLines << "移动：A键/左方向键 向左移动"
                  << "      D键/右方向键 向右移动"
                  << "技能：J键 防守技能（伤害5）"
                  << "      K键 攻击技能（伤害10）"
                  << "跳跃：W键/上方向键 单次跳跃"
                  << "      （空中可控制左右移动）"
                  << "流程：击败敌人→触发剧情"
                  << "      →击败所有敌人即可通关";

        int y = 60;
        for (const QString& line : helpLines) {
            bufferPainter.drawText(30, y, line);
            y += 14;
        }

        bufferPainter.setBrush(QColor(60, 60, 80, 200));
        bufferPainter.drawRect(78, 200, 100, 24);
        bufferPainter.drawText(78, 200, 100, 24, Qt::AlignCenter, "返回开始页面");
        bufferPainter.drawText(buffer.rect(), Qt::AlignBottom | Qt::AlignHCenter, "Esc-返回");
    } else if (m_currentPhase == Phase_GameOver) {
        font.setPointSize(16);
        bufferPainter.setFont(font);
        bufferPainter.drawText(buffer.rect(), Qt::AlignCenter, "游戏结束");
        font.setPointSize(8);
        bufferPainter.setFont(font);
        bufferPainter.drawText(buffer.rect(), Qt::AlignBottom | Qt::AlignHCenter, "按任意键退出");
    } else {
        m_player.draw(&bufferPainter);

        for (const auto& enemy : m_enemies) {
            enemy.draw(&bufferPainter);
        }

        if (m_npc) {
            m_npc->draw(&bufferPainter);
        }

        if (m_showDialog && m_npc) {
            bufferPainter.setBrush(QColor(20, 20, 40, 200));
            bufferPainter.drawRect(20, 180, 216, 60);
            bufferPainter.setPen(Qt::white);
            font.setPointSize(8);
            font.setFamily("SimHei");
            font.setStyleStrategy(QFont::NoAntialias);
            bufferPainter.setFont(font);
            bufferPainter.drawText(QRect(30, 190, 196, 40), Qt::AlignLeft | Qt::TextWordWrap, m_npc->getCurrentDialog());
            bufferPainter.drawText(200, 235, "按任意键继续");
        }

        if (m_currentPhase == Phase_Complete) {
            bufferPainter.fillRect(buffer.rect(), QColor(0, 0, 0, 200));
            font.setPointSize(16);
            font.setFamily("SimHei");
            font.setStyleStrategy(QFont::NoAntialias);
            bufferPainter.setFont(font);
            bufferPainter.drawText(buffer.rect(), Qt::AlignCenter, "恭喜通关！");
        }
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawPixmap(rect(), buffer, buffer.rect());
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (m_currentPhase == Phase_Start) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            m_currentPhase = Phase_Xieqianji;
            Enemy xieqianji(QPointF(200, 192), Enemy::Xieqianji);
            xieqianji.setAttackClip(&m_xieqianjiClip);
            m_enemies.append(xieqianji);
            update();
        } else if (event->key() == Qt::Key_H) {
            m_currentPhase = Phase_Help;
            update();
        }
        return;
    }

    if (m_currentPhase == Phase_Help) {
        if (event->key() == Qt::Key_Escape) {
            m_currentPhase = Phase_Start;
            update();
        }
        return;
    }

    if (m_currentPhase == Phase_GameOver) {
        QApplication::quit();
        return;
    }

    if (!isRunning && m_currentPhase != Phase_Suzhi_Dialog && m_currentPhase != Phase_Complete) return;

    if (m_currentPhase == Phase_Complete) {
        m_currentPhase = Phase_Start;
        isRunning = true;
        m_enemies.clear();
        return;
    }

    if (m_showDialog && m_npc) {
        m_npc->nextDialog();
        if (m_npc->isDialogFinished()) {
            m_showDialog = false;
            m_npc->endDialog();
            delete m_npc;
            m_npc = nullptr;
            m_currentPhase = Phase_Muyinzhen;
            Enemy muyinzhen(QPointF(200, 116), Enemy::Muyinzhen);
            muyinzhen.setAttackClip(&m_muyinzhenClip);
            m_enemies.append(muyinzhen);
            isStopped = false;
        }
        return;
    }

    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        m_player.setMoveLeft(true);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_player.setMoveRight(true);
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        m_player.setJump(true);
        break;
    case Qt::Key_J:
        m_player.attack(0);
        checkAttackHit(m_defenseDamage);
        break;
    case Qt::Key_K:
        m_player.attack(1);
        checkAttackHit(m_attackDamage);
        break;
    case Qt::Key_P:
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
    case Qt::Key_W:
    case Qt::Key_Up:
        m_player.setJump(false);
        break;
    }
}

void GameWidget::checkAttackHit(int damage)
{
    QRectF attackRange = m_player.getAttackRange();
    for (int i = 0; i < m_enemies.size(); ++i) {
        if (attackRange.intersects(m_enemies[i].getCollisionRect())) {
            m_enemies[i].takeDamage(damage);
            break;
        }
    }
}

void GameWidget::Init_Game()
{
    isRunning = true;
}

void GameWidget::startTimer()
{
    connect(&m_timer, &QTimer::timeout, this, &GameWidget::gameUpdate);
    m_timer.start(16);
}

void GameWidget::stopTimer()
{
    m_timer.stop();
}

void GameWidget::playerMove()
{
}

void GameWidget::handleGamePhase()
{
}

void GameWidget::showGameComplete()
{
}