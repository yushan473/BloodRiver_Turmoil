#ifndef SKILL_H
#define SKILL_H

#include <QString>

class Skill
{
public:
    QString name;
    int cooldownMs = 0;// ¿‰»¥ ±º‰
    int damage = 0;
    //AnimationClip* animationClip = nullptr;

    Skill() = default;
    Skill(const QString& n, int cdMs, int dmg)
        : name(n), cooldownMs(cdMs), damage(dmg) {}

    bool canCast() const { return currentCooldownSec <= 0.0f; }
    void cast();
    void updateCooldown(float deltaSeconds);

private:
    float currentCooldownSec = 0.0f;
};

#endif // SKILL_H
