#include "Enemy.h"
#include <QPainter>
#include <QtMath>

Enemy::Enemy(const QPointF& pos)
{
    m_transform.position = pos;
    // 简单生成一个红色矩形作为动画（暂时不用真正的动画剪辑）
    // 为了演示，我们手动创建一个单帧动画剪辑
}

void Enemy::update(float deltaSeconds, float playerX)
{
    // 简单 AI：向玩家方向移动
    if (playerX > m_transform.position.x()) m_direction = 1;
    else if (playerX < m_transform.position.x()) m_direction = -1;
    m_transform.position.rx() += m_direction * m_speed * deltaSeconds;

    // 边界限制（可选）
    // 更新动画（可省略）
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
