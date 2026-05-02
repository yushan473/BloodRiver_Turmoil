#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include "Transform.h"
#include "AnimationPlayer.h"

class Enemy :public Entity
{
public:
    Enemy(const QPointF& pos);

    void update(float deltaSeconds, const QPointF& playerPos);
    void update(float deltaSeconds)override;
    void draw(QPainter* painter) const override;

    QRectF getCollisionRect() const override;
    void takeDamage(int damage)override;
    bool isAlive() const override { return m_health > 0; }

private:
    Transform m_transform;
    AnimationPlayer m_animPlayer;
    int m_health = 30;
    float m_speed = 50.0f;
    int m_directionX = 1;
    int m_directionY = 1;
    float m_lastPlayerX = 0.0f;
    float m_lastPlayerY = 0.0f;
};

#endif // ENEMY_H
