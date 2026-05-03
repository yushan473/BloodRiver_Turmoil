#ifndef NPC_H
#define NPC_H

#include "Entity.h"
#include "Transform.h"
#include "AnimationPlayer.h"
#include "AnimationClip.h"

class NPC : public Entity
{
public:
    enum NPCType {
        Suzhi
    };

    NPC(const QPointF& pos, NPCType type);
    void setIdleClip(AnimationClip* clip);

    void update(float deltaSeconds) override;
    void draw(QPainter* painter) const override;
    QRectF getCollisionRect() const override;

    bool isPlayerNearby(const QPointF& playerPos) const;
    bool isInDialog() const { return m_inDialog; }
    void startDialog();
    void endDialog();
    const QString& getCurrentDialog() const { return m_currentDialog; }
    void nextDialog();
    bool isDialogFinished() const { return !m_inDialog; }

private:
    Transform m_transform;
    AnimationPlayer m_animPlayer;
    AnimationClip* m_idleClip = nullptr;
    NPCType m_type;
    bool m_inDialog = false;
    int m_dialogIndex = 0;
    QString m_currentDialog;
    QStringList m_dialogLines;

    void loadDialog();
};

#endif // NPC_H