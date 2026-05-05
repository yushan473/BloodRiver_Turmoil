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

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    // 设置窗口大小为 512x512 像素
    setFixedSize(512, 512);
    
    // 设置焦点策略，确保能接收键盘事件
    setFocusPolicy(Qt::StrongFocus);
    
    // 初始化游戏状态和计时器
    Init_Game();
    startTimer();
    
    // 保存全局指针
    //widget = this;

    // 加载游戏资源
    loadBackground();
    loadAnimations();

    // 设置玩家动画剪辑
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

    // 优化渲染性能
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_PaintOnScreen, false);
}

// ----------------------------------------------------------------------------
// 析构函数 - 释放资源
// ----------------------------------------------------------------------------
GameWidget::~GameWidget()
{
    delete npc;
    delete bgMusic;
    delete audioOutput;
}

// ----------------------------------------------------------------------------
// 加载游戏背景图
// ----------------------------------------------------------------------------
void GameWidget::loadBackground()
{
    background.load(":/res/image/background.png");
}

// ----------------------------------------------------------------------------
// 加载所有动画资源和UI图片
// ----------------------------------------------------------------------------
void GameWidget::loadAnimations()
{
    // ========== 玩家动画 ==========
    idleClip.loadFromSpriteSheet(":/res/image/p_idle.png", 3, 150);      // 待机动画（3帧，每帧150ms）
    walkClip.loadFromSpriteSheet(":/res/image/player_walk01.png", 8, 100); // 行走动画（8帧，每帧100ms）
    attackLv1Clip.loadFromSpriteSheet(":/res/image/player_attack_lv1.png", 14, 70); // 轻攻击（14帧，每帧70ms）
    attackLv2Clip.loadFromSpriteSheet(":/res/image/player_attack_lv2.png", 13, 77); // 重攻击（13帧，每帧77ms）

    // ========== 敌人动画 ==========
    xieqianjiClip.loadFromSpriteSheet(":/res/image/enemy_Xieqianji_attack.png", 5, 100); // 谢千机
    muyinzhenClip.loadFromSpriteSheet(":/res/image/enemy_Muyinzhen_attack.png", 5, 100); // 慕阴真
    suzeClip.loadFromSpriteSheet(":/res/image/enemy_Suze_attack.png", 5, 100);         // 苏泽

    // ========== NPC动画 ==========
    npcSuzhiClip.loadFromSpriteSheet(":/res/image/npc_Suzhi_idle.png", 3, 100); // 苏止待机
    
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

// ----------------------------------------------------------------------------
// 游戏主循环 - 每帧执行一次，更新游戏状态
// ----------------------------------------------------------------------------
void GameWidget::gameUpdate()
{
    // 计算帧间隔时间（deltaTime），用于平滑动画
    static QElapsedTimer elapsed;
    if (!elapsed.isValid()) elapsed.start();
    float deltaSec = elapsed.restart() / 1000.0f;
    
    // 限制最大帧间隔为 33ms（约30fps），避免卡顿后突然跳变
    if (deltaSec > 0.033f) deltaSec = 0.033f;

    // 如果游戏未运行或暂停，直接返回
    if (!isRunning || isStopped) return;
    
    // 如果在开始界面或帮助界面，不更新游戏逻辑
    if (gameState == PHASE_START || gameState == PHASE_HELP) return;

    // ========== 更新玩家状态 ==========
    player.update(deltaSec);

    // ========== 更新敌人状态 ==========
    for (auto& enemy : enemies) {
        enemy.update(deltaSec, player.getCollisionRect().center());
    }

    // ========== 更新NPC状态 ==========
    if (npc) {
        npc->update(deltaSec);
        
        // 检查玩家是否靠近NPC，如果是则触发对话
        if (!showDialog && npc->isPlayerNearby(player.getCollisionRect().center())) {
            showDialog = true;
            currentDialogIndex = 0;
            // 注意：这里不能设置isStopped=true，否则玩家无法移动靠近NPC
        }
    }

    // ========== 玩家受伤检测 ==========
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    for (const auto& enemy : enemies) {
        // 如果玩家和敌人碰撞
        if (enemy.getCollisionRect().intersects(player.getCollisionRect())) {
            if (!isPlayerInvincible && (currentTime - lastDamageTime) > damageCooldownMs) {
                player.takeDamage(5);
                lastDamageTime = currentTime;
                isPlayerInvincible = true;
                QTimer::singleShot(500, this, [this]() { isPlayerInvincible = false; });
            }
            break;  // 只处理第一个碰撞的敌人
        }
    }

    // ========== 检查玩家死亡 ==========
    if (!player.isAlive()) {
        gameState = PHASE_GAMEOVER;
        isStopped = true;
        return;
    }

    // ========== 检查敌人死亡，推进剧情 ==========
    // 从后往前遍历，方便删除元素
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

    // 再次检查玩家状态（防止上面的循环中玩家死亡）
    if (!player.isAlive()) {
        isRunning = false;
    }

    // 触发界面重绘
    update();
}

// ----------------------------------------------------------------------------
// 绘制函数 - 负责绘制整个游戏画面
// ----------------------------------------------------------------------------
void GameWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);  // 不需要使用事件参数

    // 创建离屏缓冲（双缓冲技术，防止画面闪烁）
    QPixmap buffer(512, 512);
    QPainter bufferPainter(&buffer);

    // ========== 绘制背景 ==========
    if (!background.isNull()) {
        // 绘制游戏背景图，缩放到512x512
        bufferPainter.drawPixmap(0, 0, background.scaled(512, 512, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    } else {
        // 如果背景图加载失败，填充深灰色
        bufferPainter.fillRect(buffer.rect(), QColor(40, 40, 40));
    }

    // 关闭平滑缩放，保持像素风格
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

    // 将缓冲内容绘制到屏幕
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawPixmap(rect(), buffer, buffer.rect());
}

// ----------------------------------------------------------------------------
// 鼠标点击事件处理
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// 键盘按键按下事件处理
// ----------------------------------------------------------------------------
void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (gameState == PHASE_START) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            gameState = PHASE_XIEQIANJI;
            Enemy xieqianji(QPointF(400, 396), ENEMY_XIEQIANJI);
            xieqianji.setAttackClip(&xieqianjiClip);
            enemies.append(xieqianji);
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
        QApplication::quit();
        return;
    }

    if (!isRunning && gameState != PHASE_SUZHI_DIALOG && gameState != PHASE_WIN) return;

    if (gameState == PHASE_WIN) {
        gameState = PHASE_START;
        isRunning = true;
        enemies.clear();
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

    // ========== 游戏操作键 ==========
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        // 向左移动
        player.setMoveLeft(true);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        // 向右移动
        player.setMoveRight(true);
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        // 跳跃
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
        // P键：暂停/继续游戏
        isStopped = !isStopped;
        break;
    }
}

// ----------------------------------------------------------------------------
// 键盘按键释放事件处理
// ----------------------------------------------------------------------------
void GameWidget::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        // 松开左移键
        player.setMoveLeft(false);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        // 松开右移键
        player.setMoveRight(false);
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        // 松开跳跃键
        player.setJump(false);
        break;
    }
}

// ----------------------------------------------------------------------------
// 检查攻击是否命中敌人
// ----------------------------------------------------------------------------
void GameWidget::checkHit(int damage)
{
    // 获取玩家的攻击范围
    QRectF attackRange = player.getAttackRange();
    
    // 遍历所有敌人，检查是否在攻击范围内
    for (int i = 0; i < enemies.size(); ++i) {
        if (attackRange.intersects(enemies[i].getCollisionRect())) {
            // 如果命中，敌人受到伤害
            enemies[i].takeDamage(damage);
            break;  // 只攻击第一个命中的敌人
        }
    }
}

// ----------------------------------------------------------------------------
// 初始化游戏状态
// ----------------------------------------------------------------------------
void GameWidget::Init_Game()
{
    isRunning = true;  // 设置游戏为运行状态
}

// ----------------------------------------------------------------------------
// 启动游戏计时器（约60fps）
// ----------------------------------------------------------------------------
void GameWidget::startTimer()
{
    // 连接计时器信号到游戏更新函数
    connect(&timer, &QTimer::timeout, this, &GameWidget::gameUpdate);
    
    // 启动计时器，每16毫秒触发一次（约60fps）
    timer.start(16);
}

// ----------------------------------------------------------------------------
// 停止游戏计时器
// ----------------------------------------------------------------------------
void GameWidget::stopTimer()
{
    timer.stop();
}

