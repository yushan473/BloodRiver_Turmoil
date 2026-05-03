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
    void update(float deltaSeconds)override;
    void draw(QPainter* painter) const override;

    QRectF getCollisionRect() const override;
    QRectF getAttackRange() const;

    int getHealth() const override { return m_health; }
    void takeDamage(int amount)override;
    bool isAlive() const override { return m_health > 0; }

    void setMoveLeft(bool pressed) { m_leftPressed = pressed; }
    void setMoveRight(bool pressed) { m_rightPressed = pressed; }
    void setJump(bool pressed) { m_jumpPressed = pressed; }
    void attack(int skillIndex = 0);

    void setClips(AnimationClip* idle, AnimationClip* walk, AnimationClip* attack1, AnimationClip* attack2);

    bool isOnGround() const { return m_isOnGround; }

private:
    Transform m_transform;
    AnimationPlayer m_animPlayer;
    std::vector<Skill> m_skills;
    int m_health = 100;
    int m_maxHealth = 100;

    float m_speed = 300.0f;
    bool m_leftPressed = false;
    bool m_rightPressed = false;
    bool m_jumpPressed = false;

    bool m_isAttacking = false;
    float m_attackTimer = 0.0f;
    int m_currentAttackSkill = 0;

    bool m_isOnGround = true;
    float m_velocityY = 0.0f;
    static constexpr float m_gravity = 700.0f;
    static constexpr float m_jumpForce = -245.0f;

    AnimationClip* m_idleClip = nullptr;
    AnimationClip* m_walkClip = nullptr;
    AnimationClip* m_attackLv1Clip = nullptr;
    AnimationClip* m_attackLv2Clip = nullptr;

    void updateAnimation(float deltaSeconds);
};

#endif // PLAYER_H