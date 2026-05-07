#include "Fragment.h"
#include <QPainter>

Fragment::Fragment(const QPointF& pos, bool high)
    : isHigh(high)
{
    transform.position = pos;
    image.load(":/res/image/fragment.png");
}

void Fragment::update(float){}

void Fragment::draw(QPainter* painter) const
{
    if (!image.isNull()) {
        int x = transform.position.x() - image.width() / 2;
        int y = transform.position.y() - image.height() / 2;
        painter->drawPixmap(x, y, image);
    }
}

QRectF Fragment::getCollisionRect() const
{
    return QRectF(transform.position.x() - 6, transform.position.y() - 6, 12, 12);
}
