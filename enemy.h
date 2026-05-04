#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include "Transform.h"
#include "AnimationPlayer.h"
#include "AnimationClip.h"
#include <QPixmap>

class Enemy :public Entity
{
public:
    enum EnemyType {
        Xieqianji,
        Muyinzhen,
        Suze
    };

    Enemy(const QPointF& pos, EnemyType type = Xieqianji);
    void setAttackClip(AnimationClip* clip);

    void update(float deltaSeconds, const QPointF& playerPos);
    void update(float deltaSeconds)override;
    void draw(QPainter* painter) const override;

    QRectF getCollisionRect() const override;
    void takeDamage(int damage)override;
    bool isAlive() const override { return m_health > 0; }

    int getMaxHealth() const { return m_maxHealth; }
    EnemyType getType() const { return m_type; }

private:
    Transform m_transform;
    AnimationPlayer m_animPlayer;
    AnimationClip* m_attackClip = nullptr;
    EnemyType m_type;
    int m_health = 30;
    int m_maxHealth = 30;
    float m_speed = 30.0f;
    int m_directionX = 1;
    int m_directionY = 1;
    float m_lastPlayerX = 0.0f;
    float m_lastPlayerY = 0.0f;

    // ÌøÔ¾ÎïÀí
    bool m_isOnGround = true;
    float m_velocityY = 0.0f;
    static constexpr float m_gravity = 800.0f;
    static constexpr float m_jumpForce = -300.0f;

    // Ãû×ÖÍ¼Æ¬
    QPixmap m_namePixmap;

    void initEnemyStats(EnemyType type);
};

#endif // ENEMY_H