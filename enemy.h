#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include "Transform.h"
#include "AnimationPlayer.h"
#include "AnimationClip.h"
#include <QPixmap>

enum {
    ENEMY_XIEQIANJI,
    ENEMY_MUYINZHEN,
    ENEMY_SUZE
};

class Enemy :public Entity
{
public:

    Enemy(const QPointF& pos, int type = ENEMY_XIEQIANJI);
    void setAttackClip(AnimationClip* clip);

    void update(float deltaSeconds)override;
    void setPlayerPosition(const QPointF& pos) { playerPos = pos; }
    void draw(QPainter* painter) const override;

    QRectF getCollisionRect() const override;
    void takeDamage(int damage)override;
    bool isAlive() const override { return health > 0; }

    int getMaxHealth() const { return maxHealth; }
    int getType() const { return type; }

private:
    Transform transform;
    AnimationPlayer animPlayer;
    AnimationClip* attackClip = nullptr;
    int type;
    int health = 30;
    int maxHealth = 30;
    float speed = 30.0f;
    int directionX = 1;
    int directionY = 1;
    float lastPlayerX = 0.0f;
    float lastPlayerY = 0.0f;

    bool isOnGround = true;
    float velocityY = 0.0f;

    QPixmap namePixmap;
    QPointF playerPos;

    void initEnemyStats(int type);
};

#endif // ENEMY_H
