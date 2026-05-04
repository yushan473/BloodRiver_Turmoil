// ============================================================================
// 游戏主窗口类 - 负责整个游戏的逻辑和渲染
// ============================================================================
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

// 全局静态指针，方便其他类访问游戏实例
GameWidget* GameWidget::widget = nullptr;

// ----------------------------------------------------------------------------
// 构造函数 - 初始化游戏窗口和所有资源
// ----------------------------------------------------------------------------
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
    widget = this;

    // 加载游戏资源
    loadBackground();
    loadAnimations();

    // 设置玩家动画剪辑
    m_player.setClips(&m_idleClip, &m_walkClip, &m_attackLv1Clip, &m_attackLv2Clip);

    // ==================== 背景音乐初始化 ====================
    // 创建播放器实例（全局唯一，游戏期间只创建一次）
    m_bgMusic = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_bgMusic->setAudioOutput(m_audioOutput);
    
    // 设置音乐文件路径
    m_bgMusic->setSource(QUrl("qrc:/res/music/bgm.mp3"));
    
    // 设置无限循环播放（全程不中断）
    m_bgMusic->setLoops(QMediaPlayer::Infinite);
    
    // 设置初始音量（50%）
    m_audioOutput->setVolume(0.5);
    
    // 开始播放（只调用一次，之后不再重复调用）
    m_bgMusic->play();

    // 加载音乐开关按钮的图标
    m_musicOnImage.load(":/res/image/music_on.png");
    m_musicOffImage.load(":/res/image/music_off.png");

    // 优化渲染性能
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_PaintOnScreen, false);
}

// ----------------------------------------------------------------------------
// 析构函数 - 释放资源
// ----------------------------------------------------------------------------
GameWidget::~GameWidget()
{
    delete m_npc;           // 释放 NPC 对象
    delete m_bgMusic;       // 释放音乐播放器
    delete m_audioOutput;   // 释放音频输出
}

// ----------------------------------------------------------------------------
// 加载游戏背景图
// ----------------------------------------------------------------------------
void GameWidget::loadBackground()
{
    m_background.load(":/res/image/background.png");
}

// ----------------------------------------------------------------------------
// 加载所有动画资源和UI图片
// ----------------------------------------------------------------------------
void GameWidget::loadAnimations()
{
    // ========== 玩家动画 ==========
    m_idleClip.loadFromSpriteSheet(":/res/image/p_idle.png", 3, 150);      // 待机动画（3帧，每帧150ms）
    m_walkClip.loadFromSpriteSheet(":/res/image/player_walk01.png", 8, 100); // 行走动画（8帧，每帧100ms）
    m_attackLv1Clip.loadFromSpriteSheet(":/res/image/player_attack_lv1.png", 14, 70); // 轻攻击（14帧，每帧70ms）
    m_attackLv2Clip.loadFromSpriteSheet(":/res/image/player_attack_lv2.png", 13, 77); // 重攻击（13帧，每帧77ms）

    // ========== 敌人动画 ==========
    m_xieqianjiClip.loadFromSpriteSheet(":/res/image/enemy_Xieqianji_attack.png", 5, 100); // 谢千机
    m_muyinzhenClip.loadFromSpriteSheet(":/res/image/enemy_Muyinzhen_attack.png", 5, 100); // 慕阴真
    m_suzeClip.loadFromSpriteSheet(":/res/image/enemy_Suze_attack.png", 5, 100);         // 苏泽

    // ========== NPC动画 ==========
    m_npcSuzhiClip.loadFromSpriteSheet(":/res/image/npc_Suzhi_idle.png", 3, 100); // 苏止待机
    
    // ========== UI界面图片 ==========
    m_startImage.load(":/res/image/start.png");     // 开始界面
    m_helpImage.load(":/res/image/help.png");       // 帮助界面
    m_endImage.load(":/res/image/end.png");         // 游戏结束界面
    m_successImage.load(":/res/image/success.png"); // 通关成功界面
    
    // ========== 对话图片（共12张）==========
    m_dialogImages.clear();
    for (int i = 1; i <= 12; ++i) {
        QString path = QString(":/res/image/d%1.png").arg(i);
        QPixmap dialogImage;
        dialogImage.load(path);
        m_dialogImages.append(dialogImage);
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
    if (m_currentPhase == Phase_Start || m_currentPhase == Phase_Help) return;

    // ========== 更新玩家状态 ==========
    m_player.update(deltaSec);

    // ========== 更新敌人状态 ==========
    for (auto& enemy : m_enemies) {
        enemy.update(deltaSec, m_player.getCollisionRect().center());
    }

    // ========== 更新NPC状态 ==========
    if (m_npc) {
        m_npc->update(deltaSec);
        
        // 检查玩家是否靠近NPC，如果是则触发对话
        if (!m_showDialog && m_npc->isPlayerNearby(m_player.getCollisionRect().center())) {
            m_showDialog = true;
            m_currentDialogIndex = 0;
            // 注意：这里不能设置isStopped=true，否则玩家无法移动靠近NPC
        }
    }

    // ========== 玩家受伤检测 ==========
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    for (const auto& enemy : m_enemies) {
        // 如果玩家和敌人碰撞
        if (enemy.getCollisionRect().intersects(m_player.getCollisionRect())) {
            // 检查是否在冷却时间内（1秒内不会再次受伤）
            if (!m_isPlayerInvincible && (currentTime - m_lastDamageTime) > m_damageCooldownMs) {
                m_player.takeDamage(5);           // 玩家受到5点伤害
                m_lastDamageTime = currentTime;   // 记录受伤时间
                m_isPlayerInvincible = true;      // 设置无敌状态
                
                // 500ms后取消无敌
                QTimer::singleShot(500, this, [this]() { m_isPlayerInvincible = false; });
            }
            break;  // 只处理第一个碰撞的敌人
        }
    }

    // ========== 检查玩家死亡 ==========
    if (!m_player.isAlive()) {
        m_currentPhase = Phase_GameOver;  // 切换到游戏结束界面
        isStopped = true;
        return;
    }

    // ========== 检查敌人死亡，推进剧情 ==========
    // 从后往前遍历，方便删除元素
    for (int i = m_enemies.size()-1; i >= 0; --i) {
        if (!m_enemies[i].isAlive()) {
            Enemy::EnemyType type = m_enemies[i].getType();
            m_enemies.removeAt(i);  // 移除死亡的敌人

            // 根据击败的敌人类型，推进到下一个阶段
            if (type == Enemy::Xieqianji && !m_xieqianjiDefeated) {
                // 击败谢千机后，进入NPC对话阶段
                m_xieqianjiDefeated = true;
                m_currentPhase = Phase_Suzhi_Dialog;
                m_npc = new NPC(QPointF(360, 396), NPC::Suzhi);
                m_npc->setIdleClip(&m_npcSuzhiClip);
            } else if (type == Enemy::Muyinzhen && !m_muyinzhenDefeated) {
                // 击败慕阴真后，进入苏泽战斗阶段
                m_muyinzhenDefeated = true;
                m_currentPhase = Phase_Suze;
                Enemy suze(QPointF(400, 396), Enemy::Suze);
                suze.setAttackClip(&m_suzeClip);
                m_enemies.append(suze);
            } else if (type == Enemy::Suze && !m_suzeDefeated) {
                // 击败苏泽后，通关成功
                m_suzeDefeated = true;
                m_currentPhase = Phase_Complete;
                isRunning = false;
            }
        }
    }

    // 再次检查玩家状态（防止上面的循环中玩家死亡）
    if (!m_player.isAlive()) {
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
    if (!m_background.isNull()) {
        // 绘制游戏背景图，缩放到512x512
        bufferPainter.drawPixmap(0, 0, m_background.scaled(512, 512, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    } else {
        // 如果背景图加载失败，填充深灰色
        bufferPainter.fillRect(buffer.rect(), QColor(40, 40, 40));
    }

    // 关闭平滑缩放，保持像素风格
    bufferPainter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    // ========== 根据游戏阶段绘制不同内容 ==========
    if (m_currentPhase == Phase_Start) {
        // 开始界面
        if (!m_startImage.isNull()) {
            bufferPainter.drawPixmap(0, 0, m_startImage);
        }
    } else if (m_currentPhase == Phase_Help) {
        // 帮助界面
        if (!m_helpImage.isNull()) {
            bufferPainter.drawPixmap(0, 0, m_helpImage);
        }
    } else if (m_currentPhase == Phase_GameOver) {
        // 游戏结束界面
        if (!m_endImage.isNull()) {
            bufferPainter.drawPixmap(0, 0, m_endImage);
        }
    } else {
        // 游戏进行中
        // 绘制玩家
        m_player.draw(&bufferPainter);

        // 绘制所有敌人
        for (const auto& enemy : m_enemies) {
            enemy.draw(&bufferPainter);
        }

        // 绘制NPC（如果存在）
        if (m_npc) {
            m_npc->draw(&bufferPainter);
        }

        // 绘制对话图片（如果正在显示对话）
        if (m_showDialog && m_currentDialogIndex < m_dialogImages.size()) {
            // 对话图片尺寸是 512x192，垂直居中显示
            int dialogY = (512 - 192) / 2;
            bufferPainter.drawPixmap(0, dialogY, m_dialogImages[m_currentDialogIndex]);
        }

        // 绘制通关成功界面
        if (m_currentPhase == Phase_Complete) {
            if (!m_successImage.isNull()) {
                bufferPainter.drawPixmap(0, 0, m_successImage);
            }
        }
    }

    // ========== 绘制音乐开关按钮（右上角）==========
    int btnX = 512 - 32;  // 右边距8像素
    int btnY = 8;         // 上边距8像素
    QPixmap btnImage = m_musicEnabled ? m_musicOnImage : m_musicOffImage;
    
    if (!btnImage.isNull()) {
        // 如果有按钮图片，绘制图片
        bufferPainter.drawPixmap(btnX, btnY, btnImage.scaled(24, 24, Qt::KeepAspectRatio));
    } else {
        // 如果没有图片，绘制简单的莫兰迪风格按钮
        bufferPainter.setBrush(m_musicEnabled ? QColor(126, 140, 132) : QColor(80, 80, 80));
        bufferPainter.drawRect(btnX, btnY, 24, 24);
        bufferPainter.setPen(Qt::white);
        bufferPainter.drawText(btnX + 6, btnY + 18, m_musicEnabled ? "?" : "?");
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
    // 检查是否点击了音乐开关按钮（右上角 24x24 的区域）
    int btnX = 512 - 32;
    int btnY = 8;
    if (event->pos().x() >= btnX && event->pos().x() <= btnX + 24 &&
        event->pos().y() >= btnY && event->pos().y() <= btnY + 24) {
        
        // 切换音乐开关状态
        m_musicEnabled = !m_musicEnabled;
        
        // 使用音量控制代替停止/播放，确保背景音乐全程不中断循环播放
        if (m_musicEnabled) {
            m_audioOutput->setVolume(0.5);  // 恢复音量到50%
        } else {
            m_audioOutput->setVolume(0.0);   // 静音
        }
        
        update();  // 更新画面显示
        return;
    }
}

// ----------------------------------------------------------------------------
// 键盘按键按下事件处理
// ----------------------------------------------------------------------------
void GameWidget::keyPressEvent(QKeyEvent* event)
{
    // ========== 开始界面 ==========
    if (m_currentPhase == Phase_Start) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            // 按回车键开始游戏，进入第一个BOSS战（谢千机）
            m_currentPhase = Phase_Xieqianji;
            Enemy xieqianji(QPointF(400, 396), Enemy::Xieqianji);
            xieqianji.setAttackClip(&m_xieqianjiClip);
            m_enemies.append(xieqianji);
            update();
        } else if (event->key() == Qt::Key_H) {
            // 按H键进入帮助界面
            m_currentPhase = Phase_Help;
            update();
        }
        return;
    }

    // ========== 帮助界面 ==========
    if (m_currentPhase == Phase_Help) {
        if (event->key() == Qt::Key_Escape) {
            // 按ESC键返回开始界面
            m_currentPhase = Phase_Start;
            update();
        }
        return;
    }

    // ========== 游戏结束界面 ==========
    if (m_currentPhase == Phase_GameOver) {
        // 按任意键退出游戏
        QApplication::quit();
        return;
    }

    // 如果游戏未运行，且不在对话或通关阶段，直接返回
    if (!isRunning && m_currentPhase != Phase_Suzhi_Dialog && m_currentPhase != Phase_Complete) return;

    // ========== 通关成功界面 ==========
    if (m_currentPhase == Phase_Complete) {
        // 按任意键返回开始界面
        m_currentPhase = Phase_Start;
        isRunning = true;
        m_enemies.clear();  // 清空敌人列表
        return;
    }

    // ========== 对话阶段 ==========
    if (m_showDialog) {
        // 按空格或回车键切换对话
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            m_currentDialogIndex++;
            
            // 如果对话全部显示完毕
            if (m_currentDialogIndex >= m_dialogImages.size()) {
                m_showDialog = false;
                m_currentDialogIndex = 0;
                
                // 删除NPC
                if (m_npc) {
                    delete m_npc;
                    m_npc = nullptr;
                }
                
                // 进入慕阴真战斗阶段
                m_currentPhase = Phase_Muyinzhen;
                Enemy muyinzhen(QPointF(400, 290), Enemy::Muyinzhen);
                muyinzhen.setAttackClip(&m_muyinzhenClip);
                m_enemies.append(muyinzhen);
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
        m_player.setMoveLeft(true);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        // 向右移动
        m_player.setMoveRight(true);
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        // 跳跃
        m_player.setJump(true);
        break;
    case Qt::Key_J:
        // J键：防守技能（轻攻击，伤害5）
        m_player.attack(0);
        checkAttackHit(m_defenseDamage);
        break;
    case Qt::Key_K:
        // K键：攻击技能（重攻击，伤害10）
        m_player.attack(1);
        checkAttackHit(m_attackDamage);
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
        m_player.setMoveLeft(false);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        // 松开右移键
        m_player.setMoveRight(false);
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        // 松开跳跃键
        m_player.setJump(false);
        break;
    }
}

// ----------------------------------------------------------------------------
// 检查攻击是否命中敌人
// ----------------------------------------------------------------------------
void GameWidget::checkAttackHit(int damage)
{
    // 获取玩家的攻击范围
    QRectF attackRange = m_player.getAttackRange();
    
    // 遍历所有敌人，检查是否在攻击范围内
    for (int i = 0; i < m_enemies.size(); ++i) {
        if (attackRange.intersects(m_enemies[i].getCollisionRect())) {
            // 如果命中，敌人受到伤害
            m_enemies[i].takeDamage(damage);
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
    connect(&m_timer, &QTimer::timeout, this, &GameWidget::gameUpdate);
    
    // 启动计时器，每16毫秒触发一次（约60fps）
    m_timer.start(16);
}

// ----------------------------------------------------------------------------
// 停止游戏计时器
// ----------------------------------------------------------------------------
void GameWidget::stopTimer()
{
    m_timer.stop();
}

// ----------------------------------------------------------------------------
// 玩家移动处理（预留接口）
// ----------------------------------------------------------------------------
void GameWidget::playerMove()
{
    // 空函数，预留用于未来扩展
}

// ----------------------------------------------------------------------------
// 游戏阶段处理（预留接口）
// ----------------------------------------------------------------------------
void GameWidget::handleGamePhase()
{
    // 空函数，预留用于未来扩展
}

// ----------------------------------------------------------------------------
// 显示通关成功界面（预留接口）
// ----------------------------------------------------------------------------
void GameWidget::showGameComplete()
{
    // 空函数，预留用于未来扩展
}
