#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QPointF>

struct Transform
{
    QPointF position;

    void translate(const QPointF& delta) { position += delta; }
    void setPosition(float x, float y) { position = {x, y}; }
};

#endif // TRANSFORM_H
