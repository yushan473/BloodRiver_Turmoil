#include "Enemy.h"
#include <QPainter>
#include <QtMath>
#include <cstdlib>

const float GRAVITY = 800.0f;
const float JUMP_FORCE = -300.0f;

Enemy::Enemy(const QPointF& pos, int t)
    : type(t)
{
    transform.position = pos;
    initEnemyStats(t);
}

void Enemy::initEnemyStats(int t)
{
    switch (t) {
    case ENEMY_XIEQIANJI:
        health = 40;
        maxHealth = 40;
        speed = 300.0f;
        namePixmap.load(":/res/image/name_xieqianji.png");
        break;
    case ENEMY_MUYINZHEN:
        health = 60;
        maxHealth = 60;
        speed = 320.0f;
        namePixmap.load(":/res/image/name_muyinzhen.png");
        break;
    case ENEMY_SUZE:
        health = 100;
        maxHealth = 100;
        speed = 320.0f;
        namePixmap.load(":/res/image/name_suze.png");
        break;
    }
}

void Enemy::setAttackClip(AnimationClip* clip)
{
    attackClip = clip;
    if (attackClip) {
        animPlayer.play(attackClip, false);
    }
}



void Enemy::update(float deltaSeconds)
{
    float dx = playerPos.x() - transform.position.x();
    float dist = qSqrt(dx * dx);

    const float minDistance = 30.0f;
    bool shouldMove = dist > minDistance;

    if (!shouldMove && dist > 5.0f && (rand() % 50) == 0) {
        shouldMove = true;
    }

    if (shouldMove) {
        directionX = (dx > 0) ? 1 : -1;
        transform.position.rx() += directionX * speed * deltaSeconds;
    }

    // float margin = 20.0f;
    // if (transform.position.x() < margin) transform.position.setX(margin);
    // if (transform.position.x() > 512 - margin) transform.position.setX(512 - margin);

    if (type != ENEMY_MUYINZHEN) {
        if (isOnGround && (rand() % 200) == 0) {
            velocityY = JUMP_FORCE;
            isOnGround = false;
        }

        if (!isOnGround) {
            velocityY += GRAVITY * deltaSeconds;
            transform.position.ry() += velocityY * deltaSeconds;

            const float groundY = 396.0f;
            if (transform.position.y() >= groundY) {
                transform.position.setY(groundY);
                velocityY = 0.0f;
                isOnGround = true;
            }
        }
    }

    lastPlayerX = playerPos.x();
    lastPlayerY = playerPos.y();

    animPlayer.update(deltaSeconds);
}

void Enemy::draw(QPainter* painter) const
{
    int drawX = transform.position.x();
    int drawY = transform.position.y();

    QPixmap frame = animPlayer.getCurrentFrame();
    if (!frame.isNull()) {
        int halfW = frame.width() / 2;
        int halfH = frame.height() / 2;
        painter->drawPixmap(drawX - halfW, drawY - halfH, frame);

        float healthPercent = (float)health / maxHealth;
        int barWidth = 32;
        int barHeight = 4;
        int barX = transform.position.x() - barWidth / 2;
        int barY = transform.position.y() - frame.height() / 2 - 8;

        if (!namePixmap.isNull()) {
            int nameX = transform.position.x() - namePixmap.width() / 2;
            int nameY = barY - namePixmap.height() - 2;
            painter->drawPixmap(nameX, nameY, namePixmap);
        }

        painter->setPen(QColor(136, 136, 136));
        painter->setBrush(QColor(58, 58, 58));
        painter->drawRect(barX, barY, barWidth, barHeight);
        painter->setBrush(QColor(126, 140, 132));
        painter->drawRect(barX, barY, barWidth * healthPercent, barHeight);
    }
}

QRectF Enemy::getCollisionRect() const
{
    QPixmap frame = animPlayer.getCurrentFrame();
    int w = frame.width() > 0 ? frame.width() : 24;
    int h = frame.height() > 0 ? frame.height() : 48;
    
    if (type == ENEMY_MUYINZHEN) {
        int attackRange = 120;
        return QRectF(transform.position.x() - w/2 - 10, transform.position.y() - h, w + 20, h + attackRange);
    }
    
    return QRectF(transform.position.x() - w/2, transform.position.y() - h, w, h);
}

void Enemy::takeDamage(int damage)
{
    health -= damage;
    if (health < 0) health = 0;
}
