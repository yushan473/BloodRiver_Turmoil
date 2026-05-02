#include "Enemy.h"
#include <QPainter>
#include <QtMath>

Enemy::Enemy(const QPointF& pos)
{
    m_transform.position = pos;
    // 简单生成一个红色矩形作为动画（暂时不用真正的动画剪辑）
    // 为了演示，我们手动创建一个单帧动画剪辑
}

void Enemy::update(float deltaSeconds, const QPointF& playerPos)
{
    float dx = playerPos.x() - m_transform.position.x();
    float dy = playerPos.y() - m_transform.position.y();
    float dist = qSqrt(dx * dx + dy * dy);
    if (dist > 1.0f) {
        m_directionX = (dx > 0) ? 1 : -1;
        m_directionY = (dy > 0) ? 1 : -1;
    }
    m_transform.position.rx() += m_directionX * m_speed * deltaSeconds;
    m_transform.position.ry() += m_directionY * m_speed * deltaSeconds;

    float margin = 20.0f;
    if (m_transform.position.x() < margin) m_transform.position.setX(margin);
    if (m_transform.position.x() > 256 - margin) m_transform.position.setX(256 - margin);
    if (m_transform.position.y() < margin) m_transform.position.setY(margin);
    if (m_transform.position.y() > 256 - margin) m_transform.position.setY(256 - margin);

    m_lastPlayerX = playerPos.x();
    m_lastPlayerY = playerPos.y();
}

void Enemy::update(float deltaSeconds)
{
    Q_UNUSED(deltaSeconds);
}

void Enemy::draw(QPainter* painter) const
{
    // 暂时用红色矩形绘制
    painter->setBrush(Qt::red);
    painter->drawRect(m_transform.position.x() - 20, m_transform.position.y() - 20, 40, 40);
}

QRectF Enemy::getCollisionRect() const
{
    return QRectF(m_transform.position.x() - 20, m_transform.position.y() - 20, 40, 40);
}

void Enemy::takeDamage(int damage)
{
    m_health -= damage;
    // 可以添加受击闪烁效果，暂时忽略
}
