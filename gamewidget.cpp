#include "GameWidget.h"
#include "Player.h"
#include "Enemy.h"
#include "NPC.h"
#include "AnimationClip.h"
#include <QPainter>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(512, 512);
    setFocusPolicy(Qt::StrongFocus);
    Init_Game();
    startTimer();
    loadBackground();
    loadAnimations();
    player.setClips(&idleClip, &walkClip, &attackLv1Clip, &attackLv2Clip, &jumpClip);

    bgMusic = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    bgMusic->setAudioOutput(audioOutput);
    bgMusic->setSource(QUrl("qrc:/res/music/bgm.mp3"));
    bgMusic->setLoops(QMediaPlayer::Infinite);
    audioOutput->setVolume(0.3);
    bgMusic->play();

    musicOnImage.load(":/res/image/music_on.png");
    musicOffImage.load(":/res/image/music_off.png");
}

GameWidget::~GameWidget()
{
    delete npc;
    delete bgMusic;
    delete audioOutput;
    for (int i = 0; i < enemies.size(); i++) {
        delete enemies[i];
    }
    enemies.clear();
    for (int i = 0; i < fragments.size(); i++) {
        delete fragments[i];
    }
    fragments.clear();
}

void GameWidget::loadBackground()
{
    background.load(":/res/image/bg_loop.png");

    if (background.isNull()) {
       // qDebug() << "bg_loop.png 加载失败！";
        background = QPixmap(1024, 512);
        background.fill(QColor(100, 150, 200));
    }
}
void GameWidget::loadAnimations()
{
    player.superClip = &superClip;
    idleClip.loadFromSpriteSheet(":/res/image/p_idle.png", 3, 150);
    walkClip.loadFromSpriteSheet(":/res/image/player_walk01.png", 8, 100);
    attackLv1Clip.loadFromSpriteSheet(":/res/image/player_attack_lv1.png", 14, 70);
    attackLv2Clip.loadFromSpriteSheet(":/res/image/player_attack_lv2.png", 13, 77);
    jumpClip.loadFromSpriteSheet(":/res/image/player_jump.png", 7, 100);
    superClip.loadFromSpriteSheet(":/res/image/player_r.png", 12, 150);
    qDebug() << "superClip 是否有效:" << superClip.isValid();


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
    if (isFrozen) {
        update();
        return;
    }

    //时间到游戏失败
    if (gameState == PHASE_INGAME && gameTimer >= 300.0f) {
        gameState = PHASE_GAMEOVER;
        update();
        isRunning = false;
        return;
    }

    static QElapsedTimer elapsed;
    if (!elapsed.isValid()) elapsed.start();
    float deltaSec = elapsed.restart() / 1000.0f;
    
    if (deltaSec > 0.033f) deltaSec = 0.033f;

    if (!isRunning || isStopped) return;
    if (gameState == PHASE_START || gameState == PHASE_HELP || gameState == PHASE_GAMEOVER) return;

    player.update(deltaSec);

    //刷怪
    if (gameState == PHASE_INGAME && !isFrozen)
    {
        spawnTimer -= deltaSec;

        //动态加快刷怪
        float progress = (float)enemies.size() / MAX_ENEMIES;
        float targetInterval = 3.0f - (3.0f - MIN_SPAWN_INTERVAL) * progress;
        spawnInterval = targetInterval;

        if (spawnTimer <= 0.0f && enemies.size() < MAX_ENEMIES)
        {
            //生成位置
            float spawnX = player.getCollisionRect().center().x() + 500 + (rand() % 300);
            float spawnY = 396.0f;

            //随机敌人类型
            int type = rand() % 3;
            if (type == ENEMY_MUYINZHEN) {
                spawnY = 320.0f;
            }

            Enemy* newEnemy = new Enemy(QPointF(spawnX, spawnY), type);

            if (type == ENEMY_XIEQIANJI) newEnemy->setAttackClip(&xieqianjiClip);
            else if (type == ENEMY_MUYINZHEN) newEnemy->setAttackClip(&muyinzhenClip);
            else if (type == ENEMY_SUZE) newEnemy->setAttackClip(&suzeClip);

            enemies.append(newEnemy);
            spawnTimer = spawnInterval;
        }
    }

    //生成碎片
    if (gameState == PHASE_INGAME && !isFrozen)
    {
        static float lastPlayerX = player.getCollisionRect().center().x();
        float currentPlayerX = player.getCollisionRect().center().x();
        float deltaDist = currentPlayerX - lastPlayerX;

        if (deltaDist > 0) {
            distanceSinceLastFragment += deltaDist;

            while (distanceSinceLastFragment >= FRAGMENT_SPAWN_DISTANCE) {
                distanceSinceLastFragment -= FRAGMENT_SPAWN_DISTANCE;

                //生成位置
                float spawnX = currentPlayerX + 200 + (rand() % 600);
                float spawnY;
                bool isHigh;

                // 概率生成高碎片（需要二段跳）
                if (rand() %100 <70) {
                    isHigh = true;
                    spawnY = 270.0f;
                } else {
                    isHigh = false;
                    spawnY = 320.0f;
                }

                Fragment* newFragment = new Fragment(QPointF(spawnX, spawnY), isHigh);
                fragments.append(newFragment);
            }
        }
        lastPlayerX = currentPlayerX;
    }

    //计时器 npc触发
    if (gameState == PHASE_INGAME && !isFrozen)
    {
        gameTimer += deltaSec;

        //检查是否到达触发时间
        if (freezeNpcIndex < 3 && gameTimer >= freezeTimes[freezeNpcIndex]) {
            freezeNpcIndex++;

            //玩家前方生成 NPC
            float npcX = player.getCollisionRect().center().x() + 300;
            npc = new NPC(QPointF(npcX, 396), NPC_SUZHI);
            npc->setIdleClip(&npcSuzhiClip);
        }

        //检查玩家是否碰到 NPC
        if (npc && npc->isPlayerNearby(player.getCollisionRect().center())) {
            isFrozen = true;
            showDialog = true;
            currentDialogIndex = 0;
        }
    }

    int targetX = player.getCollisionRect().center().x() - 256;
    cameraX = targetX;
    //最小值不低于0
    if (cameraX < 0) cameraX = 0;

    for (int i = 0; i < enemies.size(); i++) {
        enemies[i]->setPlayerPosition(player.transform.position);
        enemies[i]->update(deltaSec);
    }

    if (npc) {
        npc->update(deltaSec);
        if (!showDialog && npc->isPlayerNearby(player.getCollisionRect().center())) {
            showDialog = true;
            currentDialogIndex = 0;
        }
    }

    for (int i = 0; i < enemies.size(); i++) {
        if (!isPlayerInvincible && enemies[i]->getCollisionRect().intersects(player.getCollisionRect())) {
            player.takeDamage(5);
            isPlayerInvincible = true;
            QTimer::singleShot(500, this, &GameWidget::onInvincibilityEnd);
            break;
        }
    }

    //碎片拾取
    for (int i = fragments.size() - 1; i >= 0; i--) {
        if (fragments[i]->getCollisionRect().intersects(player.getCollisionRect())) {
            //高碎片需要二段跳才能捡
            if (fragments[i]->isHighFragment() && !player.canDoubleJump) {
                continue;  //不能捡，跳过
            }

            delete fragments[i];
            fragments.removeAt(i);
            collectedFragments++;

            qDebug() << "碎片:" << collectedFragments << "/" << TARGET_FRAGMENTS;
            break;
        }
    }

    if (!player.isAlive()) {
        gameState = PHASE_GAMEOVER;
        isRunning = false;
        enemies.clear();
    }

    for (int i = enemies.size()-1; i >= 0; --i) {
        if (!enemies[i]->isAlive()) {
            delete enemies[i];
            enemies.removeAt(i);
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

    //相机
    bufferPainter.translate(-cameraX, 0);

    if (!background.isNull()) {
        int sw = 1024;
        int startX = (cameraX / sw) * sw - sw;

        for (int x = startX; x <= cameraX + 512; x += sw) {
            bufferPainter.drawPixmap(x, 0, background);
        }
    }else {
        bufferPainter.fillRect(0, 0, 512, 512, QColor(80, 100, 150));
    }



    bufferPainter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    if (gameState == PHASE_START) {
        bufferPainter.resetTransform();
        if (!startImage.isNull()) bufferPainter.drawPixmap(0, 0, startImage);
    } else if (gameState == PHASE_HELP) {
        bufferPainter.resetTransform();
        if (!helpImage.isNull()) bufferPainter.drawPixmap(0, 0, helpImage);
    } else if (gameState == PHASE_GAMEOVER) {
        bufferPainter.resetTransform();
        if (!endImage.isNull()) bufferPainter.drawPixmap(0, 0, endImage);
    } else {
        //player
        player.draw(&bufferPainter);
        //enemy
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i]->draw(&bufferPainter);
        }
        //fragment
        for (int i = 0; i < fragments.size(); i++) {
            fragments[i]->draw(&bufferPainter);
        }
        if (npc) npc->draw(&bufferPainter);
        if (showDialog && currentDialogIndex < dialogImages.size()) {
            int dialogY = (512 - 192) / 2;
            bufferPainter.resetTransform();  //对话图片不随相机
            bufferPainter.drawPixmap(0, dialogY, dialogImages[currentDialogIndex]);
            bufferPainter.translate(-cameraX, 0);  //恢复相机变换
        }
        if (gameState == PHASE_WIN) {
            qDebug() << "绘制胜利画面，successImage是否为空:" << successImage.isNull();
            bufferPainter.resetTransform();
            if (!successImage.isNull()) bufferPainter.drawPixmap(0, 0, successImage);
        }
    }


    //显示碎片数量
    bufferPainter.resetTransform();  //确保不受相机影响
    bufferPainter.setPen(Qt::white);
    bufferPainter.setFont(QFont("SimHei", 16));
    bufferPainter.drawText(10, 30, QString("眠龙剑碎片: %1 / %2").arg(collectedFragments).arg(TARGET_FRAGMENTS));

    if (collectedFragments >= TARGET_FRAGMENTS) {
        bufferPainter.setPen(Qt::red);
        bufferPainter.drawText(10, 60, "按 R 释放十八剑阵通关！");
    }

    //倒计时
    int remaining = 300 - (int)gameTimer;
    int minutes = remaining / 60;
    int seconds = remaining % 60;

    bufferPainter.resetTransform();
    bufferPainter.setPen(Qt::white);
    bufferPainter.setFont(QFont("SimHei", 16));
    bufferPainter.drawText(512 / 2 - 40, 30, QString("时间: %1:%2")
         .arg(minutes, 2, 10, QChar('0'))
         .arg(seconds, 2, 10, QChar('0')));

    //时间到游戏失败
    if (remaining <= 0 && gameState == PHASE_INGAME) {
        gameState = PHASE_GAMEOVER;
        isRunning = false;
    }

    //UI按钮（不受相机影响
    bufferPainter.resetTransform();
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
            gameState = PHASE_INGAME;
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
                isFrozen = false;
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
    case Qt::Key_R:
        if (collectedFragments >= TARGET_FRAGMENTS) {
             player.playSuperAnimation();
            QTimer::singleShot(1500, this, [this]() {
                for (int i = 0; i < enemies.size(); i++) {
                    delete enemies[i];
                }
                enemies.clear();
                gameState = PHASE_WIN;
                isRunning = false;
                update();
            });
        }
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
        if (attackRange.intersects(enemies[i]->getCollisionRect())) {
            enemies[i]->takeDamage(damage);
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

    for (int i = 0; i < enemies.size(); i++) {
        delete enemies[i];
    }
    enemies.clear();

    for (int i = 0; i < fragments.size(); i++) {
        delete fragments[i];
    }
    fragments.clear();
    collectedFragments = 0;
    distanceSinceLastFragment = 0.0f;

    gameTimer = 0.0f;
    freezeNpcIndex = 0;
    isFrozen = false;

    spawnTimer = 0.0f;
    spawnInterval = 3.0f;

    player.takeDamage(-200);
    player.setPosition(QPointF(100, 396));
    cameraX = 0;//重置相机
    repaint();
}
