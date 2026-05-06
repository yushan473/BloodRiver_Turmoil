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

    int getHealth() const override { return health; }
    void takeDamage(int amount)override;
    bool isAlive() const override { return health > 0; }

    void setMoveLeft(bool pressed) { leftPressed = pressed; }
    void setMoveRight(bool pressed) { rightPressed = pressed; }
    void setJump(bool pressed) { jumpPressed = pressed; }
    void attack(int skillIndex = 0);

    void setClips(AnimationClip* idle, AnimationClip* walk, AnimationClip* attack1, AnimationClip* attack2);
    void setPosition(const QPointF& pos) { transform.position = pos; }

    bool isOnGround() const { return onGround; }
    Transform transform;

private:
    AnimationPlayer animPlayer;
    std::vector<Skill> skills;
    int health = 100;
    int maxHealth = 100;

    float speed = 300.0f;
    bool leftPressed = false;
    bool rightPressed = false;
    bool jumpPressed = false;

    bool isAttacking = false;
    float attackTimer = 0.0f;
    int currentAttackSkill = 0;

    bool onGround = true;
    float velocityY = 0.0f;

    AnimationClip* idleClip = nullptr;
    AnimationClip* walkClip = nullptr;
    AnimationClip* attackLv1Clip = nullptr;
    AnimationClip* attackLv2Clip = nullptr;
};

#endif // PLAYER_H
