#ifndef ENTITY_H
#define ENTITY_H

#include <QRectF>
#include <QPainter>

class Entity
{
public:
    virtual ~Entity() = default;

    // ´¿Ðéº¯Êý
    virtual void update(float deltaSeconds) = 0;
    virtual void draw(QPainter* painter) const = 0;
    virtual QRectF getCollisionRect() const = 0;

    virtual bool isAlive() const { return health > 0; }
    virtual void takeDamage(int amount) { health -= amount; }
    virtual int getHealth() const { return health; }

protected:
    int health = 0;
};

#endif // ENTITY_H
