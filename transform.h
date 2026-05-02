#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QPointF>

struct Transform
{
    QPointF position;
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    void translate(const QPointF& delta) { position += delta; }
    void setPosition(float x, float y) { position = {x, y}; }
};

#endif // TRANSFORM_H
