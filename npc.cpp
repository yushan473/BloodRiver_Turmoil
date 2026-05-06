#include "NPC.h"
#include "animationclip.h"
#include <QPainter>
#include <QtMath>

NPC::NPC(const QPointF& pos, int t)
    : type(t)
{
    transform.position = pos;
}

void NPC::setIdleClip(AnimationClip* clip)
{
    idleClip = clip;
    if (idleClip) {
        animPlayer.play(idleClip, true);
    }
}

bool NPC::isPlayerNearby(const QPointF& playerPos) const
{
    float dx = playerPos.x() - transform.position.x();
    float dy = playerPos.y() - transform.position.y();
    return (dx * dx + dy * dy) < 900;
}

void NPC::update(float deltaSeconds)
{
    animPlayer.update(deltaSeconds);
}

void NPC::draw(QPainter* painter) const
{
    QPixmap frame = animPlayer.getCurrentFrame();
    if (!frame.isNull()) {
        int x = transform.position.x() - frame.width() / 2;
        int y = transform.position.y() - frame.height();
        painter->drawPixmap(x, y, frame);
    }
}

QRectF NPC::getCollisionRect() const
{
    return QRectF(transform.position.x() - 16, transform.position.y() - 48, 32, 48);
}
