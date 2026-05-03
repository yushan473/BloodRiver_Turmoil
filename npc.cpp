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

void NPC::startDialog()
{
    loadDialog();
    m_dialogIndex = 0;
    m_currentDialog = m_dialogLines.value(0, "");
    m_inDialog = true;
}

void NPC::nextDialog()
{
    if (m_dialogIndex < m_dialogLines.size() - 1) {
        m_dialogIndex++;
        m_currentDialog = m_dialogLines.value(m_dialogIndex, "");
    } else {
        m_inDialog = false;
    }
}

void NPC::endDialog()
{
    m_inDialog = false;
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

void NPC::loadDialog()
{
    switch (m_type) {
    case Suzhi:
        m_dialogLines << "苏止：你来了...很久没见过活人了。"
                      << "苏止：血河之乱后，这里就只剩怪物了。"
                      << "苏止：小心，前面有更危险的东西。"
                      << "苏止：祝你好运。";
        break;
    }
}
