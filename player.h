#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Transform.h"
#include "AnimationPlayer.h"
#include "Skill.h"
#include <vector>

class AnimationClip;

class Player : public Entity
{
public:
    Player();
    //entity接口
    void update(float deltaSeconds)override;
    void draw(QPainter* painter) const override;

    // 碰撞箱（用于攻击判定）
    QRectF getCollisionRect() const override;

    // 属性
    int getHealth() const override { return m_health; }
    void takeDamage(int amount)override;
    bool isAlive() const override { return m_health > 0; }

    //玩家接口
    // 输入控制
    void setMoveLeft(bool pressed) { m_leftPressed = pressed; }
    void setMoveRight(bool pressed) { m_rightPressed = pressed; }
    void attack();   // 使用第一个技能
    // 设置动画资源（由 GameWidget 在初始化时提供）
    void setClips(AnimationClip* idle, AnimationClip* walk, AnimationClip* attack);

private:
    Transform m_transform;
    AnimationPlayer m_animPlayer;
    std::vector<Skill> m_skills;
    int m_health = 100;
    int m_maxHealth = 100;

    // 移动参数
    float m_speed = 300.0f;   // 像素/秒
    bool m_leftPressed = false;
    bool m_rightPressed = false;

    // 攻击状态
    bool m_isAttacking = false;
    float m_attackTimer = 0.0f;

    // 动画剪辑指针（由外部传入，资源可共享）
    AnimationClip* m_idleClip = nullptr;
    AnimationClip* m_walkClip = nullptr;
    AnimationClip* m_attackClip = nullptr;

    // 内部更新动画状态
    void updateAnimation(float deltaSeconds);
};

#endif // PLAYER_H
