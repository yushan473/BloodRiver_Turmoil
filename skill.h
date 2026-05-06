#ifndef SKILL_H
#define SKILL_H

#include <QString>
#include "AnimationClip.h"

class Skill
{
public:
    QString name;
    int cooldownMs = 0;        // 冷却时间（毫秒）
    int damage = 0;
    AnimationClip* animationClip = nullptr;   // 可选，技能释放时的动画

    Skill() = default;
    Skill(const QString& n, int cdMs, int dmg, AnimationClip* clip = nullptr)
        : name(n), cooldownMs(cdMs), damage(dmg), animationClip(clip) {}

    bool canCast() const { return currentCooldownSec <= 0.0f; }
    void cast();
    void updateCooldown(float deltaSeconds);

private:
    float currentCooldownSec = 0.0f;
};

#endif // SKILL_H
