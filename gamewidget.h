// ============================================================================
// 游戏主窗口类头文件
// 负责整个游戏的逻辑控制、渲染和用户输入处理
// ============================================================================
#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QList>
#include <QApplication>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "Player.h"
#include "Enemy.h"
#include "NPC.h"
#include "AnimationClip.h"

class Player;

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    // 构造函数/析构函数
    GameWidget(QWidget *parent = nullptr);
    ~GameWidget();
    
    // 游戏控制函数
    void Init_Game();           // 初始化游戏状态
    void startTimer();          // 启动游戏主循环计时器
    void stopTimer();           // 停止游戏主循环计时器
    
    // 游戏状态标志（公开访问）
    bool isRunning = 0;         // 游戏是否正在运行
    bool isChoosing = 0;        // 是否在选择状态
    bool isStopped = 0;         // 游戏是否暂停
    bool isPressed = 0;         // 是否有按键按下
    
    // 事件处理函数
    void keyPressEvent(QKeyEvent* event) override;      // 键盘按下事件
    void keyReleaseEvent(QKeyEvent* event) override;    // 键盘释放事件
    void paintEvent(QPaintEvent* event) override;       // 绘制事件
    void mousePressEvent(QMouseEvent *event) override;  // 鼠标点击事件
    
    void playerMove();          // 玩家移动处理（预留接口）
    
    static GameWidget* widget;  // 全局静态指针，方便其他类访问

private slots:
    void gameUpdate();          // 游戏主循环（每帧更新）

private:
    // ========== 游戏实体 ==========
    Player m_player;            // 玩家对象
    QList<Enemy> m_enemies;     // 敌人生成列表
    NPC* m_npc = nullptr;       // NPC对象（对话用）
    QPixmap m_background;       // 游戏背景图

    // ========== 动画剪辑 ==========
    // 玩家动画
    AnimationClip m_idleClip;       // 待机动画
    AnimationClip m_walkClip;       // 行走动画
    AnimationClip m_attackLv1Clip;  // 轻攻击动画
    AnimationClip m_attackLv2Clip;  // 重攻击动画
    
    // 敌人动画
    AnimationClip m_xieqianjiClip;  // 谢千机动画
    AnimationClip m_muyinzhenClip;  // 慕阴真动画
    AnimationClip m_suzeClip;       // 苏泽动画
    
    // NPC动画
    AnimationClip m_npcSuzhiClip;   // 苏止待机

    // ========== 计时器 ==========
    QTimer m_timer;             // 游戏主循环计时器

    // ========== 游戏阶段枚举 ==========
    enum GamePhase {
        Phase_Start,            // 开始界面
        Phase_Help,             // 帮助界面
        Phase_Xieqianji,        // 谢千机战斗
        Phase_Suzhi_Dialog,     // 苏止对话
        Phase_Muyinzhen,        // 慕阴真战斗
        Phase_Suze,             // 苏泽战斗
        Phase_Complete,         // 通关成功
        Phase_GameOver          // 游戏结束
    };
    GamePhase m_currentPhase = Phase_Start;  // 当前游戏阶段
    
    // ========== 剧情进度标志 ==========
    bool m_xieqianjiDefeated = false;  // 谢千机是否被击败
    bool m_muyinzhenDefeated = false;  // 慕阴真是否被击败
    bool m_suzeDefeated = false;       // 苏泽是否被击败
    
    // ========== 对话系统 ==========
    bool m_showDialog = false;         // 是否显示对话
    int m_currentDialogIndex = 0;      // 当前对话索引
    
    // ========== 玩家受伤冷却 ==========
    bool m_isPlayerInvincible = false;    // 玩家是否无敌
    qint64 m_lastDamageTime = 0;          // 上次受伤时间
    const int m_damageCooldownMs = 1000;  // 受伤冷却时间（毫秒）
    
    // ========== UI图片 ==========
    QPixmap m_startImage;       // 开始界面图片
    QPixmap m_helpImage;        // 帮助界面图片
    QPixmap m_endImage;         // 游戏结束图片
    QPixmap m_successImage;     // 通关成功图片
    QList<QPixmap> m_dialogImages;  // 对话图片列表（共12张）

    // ========== 技能伤害数值 ==========
    const int m_defenseDamage = 5;   // 防守技能伤害（J键）
    const int m_attackDamage = 10;   // 攻击技能伤害（K键）

    // ========== 背景音乐系统 ==========
    QMediaPlayer* m_bgMusic = nullptr;    // 音乐播放器（全局唯一）
    QAudioOutput* m_audioOutput = nullptr; // 音频输出
    bool m_musicEnabled = true;           // 音乐是否开启
    
    // 音乐开关按钮图片
    QPixmap m_musicOnImage;     // 音乐开启图标
    QPixmap m_musicOffImage;    // 音乐关闭图标

    // ========== 辅助函数 ==========
    void loadAnimations();      // 加载所有动画资源
    void loadBackground();      // 加载背景图
    void checkAttackHit(int damage);  // 检查攻击是否命中
    void handleGamePhase();     // 游戏阶段处理（预留接口）
    void showGameComplete();    // 显示通关界面（预留接口）
};

#endif // GAMEWIDGET_H