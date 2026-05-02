#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include "Transform.h"
#include "AnimationPlayer.h"

class Enemy :public Entity
{
public:
    Enemy(const QPointF& pos);

    void update(float deltaSeconds, float playerX);
    void update(float deltaSeconds)override;
    void draw(QPainter* painter) const override;

    QRectF getCollisionRect() const override;
    void takeDamage(int damage)override;
    bool isAlive() const override { return m_health > 0; }

private:
    Transform m_transform;
    AnimationPlayer m_animPlayer;
    int m_health = 30;
    float m_speed = 80.0f;   // 移动速度
    int m_direction = 1;      // 1右 -1左
};

#endif // ENEMY_H
