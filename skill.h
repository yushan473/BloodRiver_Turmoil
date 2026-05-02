#ifndef SKILL_H
#define SKILL_H

#include <QString>

class AnimationClip;   // 前向声明

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

    bool canCast() const { return m_currentCooldownSec <= 0.0f; }
    void cast();    // 实际使用时，需要传入目标等参数，这里先空着
    void updateCooldown(float deltaSeconds);

private:
    float m_currentCooldownSec = 0.0f;
};

#endif // SKILL_H
