#ifndef ENTITY_H
#define ENTITY_H

#include <QRectF>
#include <QPainter>

class Entity
{
public:
    virtual ~Entity() = default;

    // 纯虚函数：所有实体必须实现
    virtual void update(float deltaSeconds) = 0;
    virtual void draw(QPainter* painter) const = 0;
    virtual QRectF getCollisionRect() const = 0;

    // 可选：通用属性（子类可覆盖）
    virtual bool isAlive() const { return m_health > 0; }
    virtual void takeDamage(int amount) { m_health -= amount; }
    virtual int getHealth() const { return m_health; }

protected:
    int m_health = 0;
};

#endif // ENTITY_H
