#include "GameWidget.h"
#include "Player.h"
#include "Enemy.h"
#include "NPC.h"
#include "AnimationClip.h"
#include "AnimationPlayer.h"
#include <QPainter>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QDebug>
#include <QRandomGenerator>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(512, 512);
    setFocusPolicy(Qt::StrongFocus);
    Init_Game();
    startTimer();
    loadBackground();
    loadAnimations();
    player.setClips(&idleClip, &walkClip, &attackLv1Clip, &attackLv2Clip);

    bgMusic = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    bgMusic->setAudioOutput(audioOutput);
    bgMusic->setSource(QUrl("qrc:/res/music/bgm.mp3"));
    bgMusic->setLoops(QMediaPlayer::Infinite);
    audioOutput->setVolume(0.5);
    bgMusic->play();

    musicOnImage.load(":/res/image/music_on.png");
    musicOffImage.load(":/res/image/music_off.png");
}

GameWidget::~GameWidget()
{
    delete npc;
    delete bgMusic;
    delete audioOutput;
}

void GameWidget::loadBackground()
{
    background.load(":/res/image/background.png");
}

void GameWidget::loadAnimations()
{
    idleClip.loadFromSpriteSheet(":/res/image/p_idle.png", 3, 150);
    walkClip.loadFromSpriteSheet(":/res/image/player_walk01.png", 8, 100);
    attackLv1Clip.loadFromSpriteSheet(":/res/image/player_attack_lv1.png", 14, 70);
    attackLv2Clip.loadFromSpriteSheet(":/res/image/player_attack_lv2.png", 13, 77);

    xieqianjiClip.loadFromSpriteSheet(":/res/image/enemy_Xieqianji_attack.png", 5, 100);
    muyinzhenClip.loadFromSpriteSheet(":/res/image/enemy_Muyinzhen_attack.png", 5, 100);
    suzeClip.loadFromSpriteSheet(":/res/image/enemy_Suze_attack.png", 5, 100);

    npcSuzhiClip.loadFromSpriteSheet(":/res/image/npc_Suzhi_idle.png", 3, 100);
    
    startImage.load(":/res/image/start.png");
    helpImage.load(":/res/image/help.png");
    endImage.load(":/res/image/end.png");
    successImage.load(":/res/image/success.png");
    
    dialogImages.clear();
    for (int i = 1; i <= 12; ++i) {
        QString path = QString(":/res/image/d%1.png").arg(i);
        QPixmap dialogImage;
        dialogImage.load(path);
        dialogImages.append(dialogImage);
    }
}

void GameWidget::gameUpdate()
{
    static QElapsedTimer elapsed;
    if (!elapsed.isValid()) elapsed.start();
    float deltaSec = elapsed.restart() / 1000.0f;
    
    if (deltaSec > 0.033f) deltaSec = 0.033f;

    if (!isRunning || isStopped) return;
    if (gameState == PHASE_START || gameState == PHASE_HELP || gameState == PHASE_GAMEOVER) return;

    player.update(deltaSec);

    for (auto& enemy : enemies) {
        enemy.setPlayerPosition(player.transform.position);
        enemy.update(deltaSec);
    }
    if (npc) {
        npc->update(deltaSec);
        if (!showDialog && npc->isPlayerNearby(player.getCollisionRect().center())) {
            showDialog = true;
            currentDialogIndex = 0;
        }
    }

    for (const auto& enemy : enemies) {
        if (!isPlayerInvincible && enemy.getCollisionRect().intersects(player.getCollisionRect())) {
            player.takeDamage(5);
            isPlayerInvincible = true;
            QTimer::singleShot(500, this, &GameWidget::onInvincibilityEnd);
            break;
        }
    }

    if (!player.isAlive()) {
        gameState = PHASE_GAMEOVER;
        isRunning = false;
        enemies.clear();
    }

    for (int i = enemies.size()-1; i >= 0; --i) {
        if (!enemies[i].isAlive()) {
            int type = enemies[i].getType();
            enemies.removeAt(i);

            if (type == ENEMY_XIEQIANJI && !xieqianjiDefeated) {
                xieqianjiDefeated = true;
                gameState = PHASE_SUZHI_DIALOG;
                npc = new NPC(QPointF(360, 396), NPC_SUZHI);
                npc->setIdleClip(&npcSuzhiClip);
            } else if (type == ENEMY_MUYINZHEN && !muyinzhenDefeated) {
                muyinzhenDefeated = true;
                gameState = PHASE_SUZE;
                Enemy suze(QPointF(400, 396), ENEMY_SUZE);
                suze.setAttackClip(&suzeClip);
                enemies.append(suze);
            } else if (type == ENEMY_SUZE && !suzeDefeated) {
                suzeDefeated = true;
                gameState = PHASE_WIN;
                isRunning = false;
            }
        }
    }
    update();
}

void GameWidget::onInvincibilityEnd()
{
    isPlayerInvincible = false;
}

void GameWidget::paintEvent(QPaintEvent* event)
{

    QPixmap buffer(512, 512);
    QPainter bufferPainter(&buffer);

    if (!background.isNull()) {
        bufferPainter.drawPixmap(0, 0, background.scaled(512, 512, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    } else {
        bufferPainter.fillRect(buffer.rect(), QColor(40, 40, 40));
    }

    bufferPainter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    if (gameState == PHASE_START) {
        if (!startImage.isNull()) {
            bufferPainter.drawPixmap(0, 0, startImage);
        }
    } else if (gameState == PHASE_HELP) {
        if (!helpImage.isNull()) {
            bufferPainter.drawPixmap(0, 0, helpImage);
        }
    } else if (gameState == PHASE_GAMEOVER) {
        if (!endImage.isNull()) {
            bufferPainter.drawPixmap(0, 0, endImage);
        }
    } else {
        player.draw(&bufferPainter);

        for (const auto& enemy : enemies) {
            enemy.draw(&bufferPainter);
        }

        if (npc) {
            npc->draw(&bufferPainter);
        }

        if (showDialog && currentDialogIndex < dialogImages.size()) {
            int dialogY = (512 - 192) / 2;
            bufferPainter.drawPixmap(0, dialogY, dialogImages[currentDialogIndex]);
        }

        if (gameState == PHASE_WIN) {
            if (!successImage.isNull()) {
                bufferPainter.drawPixmap(0, 0, successImage);
            }
        }
    }

    int btnX = 512 - 32;
    int btnY = 8;
    QPixmap btnImage = musicEnabled ? musicOnImage : musicOffImage;
    
    if (!btnImage.isNull()) {
        bufferPainter.drawPixmap(btnX, btnY, btnImage.scaled(24, 24, Qt::KeepAspectRatio));
    } else {
        bufferPainter.setBrush(musicEnabled ? QColor(126, 140, 132) : QColor(80, 80, 80));
        bufferPainter.drawRect(btnX, btnY, 24, 24);
        bufferPainter.setPen(Qt::white);
        bufferPainter.drawText(btnX + 6, btnY + 18, musicEnabled ? "?" : "?");
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawPixmap(rect(), buffer, buffer.rect());
}

void GameWidget::mousePressEvent(QMouseEvent* event)
{
    int btnX = 512 - 32;
    int btnY = 8;
    if (event->pos().x() >= btnX && event->pos().x() <= btnX + 24 &&
        event->pos().y() >= btnY && event->pos().y() <= btnY + 24) {
        musicEnabled = !musicEnabled;
        if (musicEnabled) {
            audioOutput->setVolume(0.5);
        } else {
            audioOutput->setVolume(0.0);
        }
        update();
        return;
    }
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (gameState == PHASE_START) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            gameState = PHASE_XIEQIANJI;
            Enemy xieqianji(QPointF(400, 396), ENEMY_XIEQIANJI);
            xieqianji.setAttackClip(&xieqianjiClip);
            enemies.append(xieqianji);
            isRunning = true;
            update();
        } else if (event->key() == Qt::Key_H) {
            gameState = PHASE_HELP;
            update();
        }
        return;
    }

    if (gameState == PHASE_HELP) {
        if (event->key() == Qt::Key_Escape) {
            gameState = PHASE_START;
            update();
        }
        return;
    }

    if (gameState == PHASE_GAMEOVER) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            resetToStart();
        }
        return;
    }
    if (!isRunning && gameState != PHASE_SUZHI_DIALOG && gameState != PHASE_WIN) return;

    if (gameState == PHASE_WIN) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            resetToStart();
        }
        return;
    }

    if (showDialog) {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            currentDialogIndex++;
            if (currentDialogIndex >= dialogImages.size()) {
                showDialog = false;
                currentDialogIndex = 0;
                if (npc) {
                    delete npc;
                    npc = nullptr;
                }
                gameState = PHASE_MUYINZHEN;
                Enemy muyinzhen(QPointF(400, 290), ENEMY_MUYINZHEN);
                muyinzhen.setAttackClip(&muyinzhenClip);
                enemies.append(muyinzhen);
                isStopped = false;
            }
        }
        return;
    }

    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        player.setMoveLeft(true);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        player.setMoveRight(true);
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        player.setJump(true);
        break;
    case Qt::Key_J:
        player.attack(0);
        checkHit(defenseDamage);
        break;
    case Qt::Key_K:
        player.attack(1);
        checkHit(attackDamage);
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
        player.setMoveLeft(false);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        player.setMoveRight(false);
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        player.setJump(false);
        break;
    }
}

void GameWidget::checkHit(int damage)
{
    QRectF attackRange = player.getAttackRange();
    
    for (int i = 0; i < enemies.size(); ++i) {
        if (attackRange.intersects(enemies[i].getCollisionRect())) {
            enemies[i].takeDamage(damage);
            break;//先只打一个
        }
    }
}

void GameWidget::Init_Game()
{
    isRunning = true;
}

void GameWidget::startTimer()
{
    connect(&timer, &QTimer::timeout, this, &GameWidget::gameUpdate);
    timer.start(16);
}

void GameWidget::stopTimer()
{
    timer.stop();
}

void GameWidget::resetToStart()
{
    gameState = PHASE_START;
    isRunning = false;
    enemies.clear();
    player.takeDamage(-100);  // 恢复满血
    xieqianjiDefeated = false;
    muyinzhenDefeated = false;
    suzeDefeated = false;
    player.setPosition(QPointF(100, 396));
    repaint();
}

