#include "NPC.h"
#include "animationclip.h"
#include <QPainter>
#include <QtMath>

NPC::NPC(const QPointF& pos, NPCType type)
    : m_type(type)
{
    m_transform.position = pos;
}

void NPC::setIdleClip(AnimationClip* clip)
{
    m_idleClip = clip;
    if (m_idleClip) {
        m_idleClip->loadFromSpriteSheet(":/res/image/npc_Suzhi_idle.png", 3, 100);
        m_animPlayer.play(m_idleClip, true);
    }
}

bool NPC::isPlayerNearby(const QPointF& playerPos) const
{
    float dx = playerPos.x() - m_transform.position.x();
    float dy = playerPos.y() - m_transform.position.y();
    return (dx * dx + dy * dy) < 900;
}

void NPC::update(float deltaSeconds)
{
    Q_UNUSED(deltaSeconds);
    m_animPlayer.update(deltaSeconds);
}

void NPC::draw(QPainter* painter) const
{
    QPixmap frame = m_animPlayer.getCurrentFrame();
    if (!frame.isNull()) {
        int x = m_transform.position.x() - frame.width() / 2;
        int y = m_transform.position.y() - frame.height();
        painter->drawPixmap(x, y, frame);
    }
}

QRectF NPC::getCollisionRect() const
{
    return QRectF(m_transform.position.x() - 16, m_transform.position.y() - 48, 32, 48);
}
