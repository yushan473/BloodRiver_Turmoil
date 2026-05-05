#ifndef NPC_H
#define NPC_H

#include "Entity.h"
#include "Transform.h"
#include "AnimationPlayer.h"
#include "AnimationClip.h"

enum {
    NPC_SUZHI
};

class NPC : public Entity
{
public:

    NPC(const QPointF& pos, int type);
    void setIdleClip(AnimationClip* clip);

    void update(float deltaSeconds) override;
    void draw(QPainter* painter) const override;
    QRectF getCollisionRect() const override;

    bool isPlayerNearby(const QPointF& playerPos) const;

private:
    Transform transform;
    AnimationPlayer animPlayer;
    AnimationClip* idleClip = nullptr;
    int type;
};

#endif // NPC_H