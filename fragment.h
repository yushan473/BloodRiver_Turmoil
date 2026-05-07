#ifndef FRAGMENT_H
#define FRAGMENT_H
#include "Entity.h"
#include "Transform.h"

class Fragment : public Entity
{
public:
    Fragment(const QPointF& pos, bool isHigh);

    void update(float deltaSeconds) override;
    void draw(QPainter* painter) const override;
    QRectF getCollisionRect() const override;

    bool isHighFragment() const { return isHigh; }
    bool isAlive() const override { return true; }

private:
    Transform transform;
    bool isHigh;
    QPixmap image;
};

#endif // FRAGMENT_H
