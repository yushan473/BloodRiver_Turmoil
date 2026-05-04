#include "Enemy.h"
#include <QPainter>
#include <QtMath>
#include <cstdlib>

Enemy::Enemy(const QPointF& pos, EnemyType type)
    : m_type(type)
{
    m_transform.position = pos;
    initEnemyStats(type);
}

void Enemy::initEnemyStats(EnemyType type)
{
    switch (type) {
    case Xieqianji:
        m_health = 20;
        m_maxHealth = 20;
        m_speed = 40.0f;
        m_namePixmap.load(":/res/image/name_xieqianji.png");
        break;
    case Muyinzhen:
        m_health = 50;
        m_maxHealth = 50;
        m_speed = 40.0f;
        m_namePixmap.load(":/res/image/name_muyinzhen.png");
        break;
    case Suze:
        m_health = 100;
        m_maxHealth = 100;
        m_speed = 50.0f;
        m_namePixmap.load(":/res/image/name_suze.png");
        break;
    }
}

void Enemy::setAttackClip(AnimationClip* clip)
{
    m_attackClip = clip;
    if (m_attackClip) {
        m_animPlayer.play(m_attackClip, true);
    }
}

void Enemy::update(float deltaSeconds, const QPointF& playerPos)
{
    float dx = playerPos.x() - m_transform.position.x();
    float dist = qSqrt(dx * dx);

    // 敌人主动向玩家移动，不受最小距离限制
    // 增加随机移动因素，让敌人看起来更主动
    const float minDistance = 30.0f;
    bool shouldMove = dist > minDistance;
    
    // 即使玩家不动，敌人也有一定概率主动移动
    if (!shouldMove && dist > 5.0f && (rand() % 50) == 0) {
        shouldMove = true;
    }
    
    if (shouldMove) {
        m_directionX = (dx > 0) ? 1 : -1;
        m_transform.position.rx() += m_directionX * m_speed * deltaSeconds;
    }

    float margin = 20.0f;
    if (m_transform.position.x() < margin) m_transform.position.setX(margin);
    if (m_transform.position.x() > 512 - margin) m_transform.position.setX(512 - margin);

    if (m_type != Muyinzhen) {
        if (m_isOnGround && (rand() % 200) == 0) {
            m_velocityY = m_jumpForce;
            m_isOnGround = false;
        }

        if (!m_isOnGround) {
            m_velocityY += m_gravity * deltaSeconds;
            m_transform.position.ry() += m_velocityY * deltaSeconds;

            const float groundY = 396.0f;  // 512画布下的地面位置
            if (m_transform.position.y() >= groundY) {
                m_transform.position.setY(groundY);
                m_velocityY = 0.0f;
                m_isOnGround = true;
            }
        }
    }

    m_lastPlayerX = playerPos.x();
    m_lastPlayerY = playerPos.y();

    m_animPlayer.update(deltaSeconds);
}

void Enemy::update(float deltaSeconds)
{
    Q_UNUSED(deltaSeconds);
}

void Enemy::draw(QPainter* painter) const
{
    int drawX = m_transform.position.x();
    int drawY = m_transform.position.y();

    QPixmap frame = m_animPlayer.getCurrentFrame();
    if (!frame.isNull()) {
        int halfW = frame.width() / 2;
        int halfH = frame.height() / 2;
        painter->drawPixmap(drawX - halfW, drawY - halfH, frame);

        float healthPercent = (float)m_health / m_maxHealth;
        int barWidth = 32;
        int barHeight = 4;
        int barX = m_transform.position.x() - barWidth / 2;
        int barY = m_transform.position.y() - frame.height() / 2 - 8;

        // 绘制名字图片（在血条上方）
        if (!m_namePixmap.isNull()) {
            int nameX = m_transform.position.x() - m_namePixmap.width() / 2;
            int nameY = barY - m_namePixmap.height() - 2;
            painter->drawPixmap(nameX, nameY, m_namePixmap);
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
    QPixmap frame = m_animPlayer.getCurrentFrame();
    int w = frame.width() > 0 ? frame.width() : 24;
    int h = frame.height() > 0 ? frame.height() : 48;
    
    // 慕阴真浮空时，需要向下延伸攻击范围才能打到地面玩家
    if (m_type == Muyinzhen) {
        // 向下延伸攻击范围，让慕阴真能够攻击到地面的玩家
        int attackRange = 120;  // 向下延伸120像素的攻击范围
        return QRectF(m_transform.position.x() - w/2 - 10, m_transform.position.y() - h, w + 20, h + attackRange);
    }
    
    return QRectF(m_transform.position.x() - w/2, m_transform.position.y() - h, w, h);
}

void Enemy::takeDamage(int damage)
{
    m_health -= damage;
    if (m_health < 0) m_health = 0;
}
