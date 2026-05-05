#include "Skill.h"

void Skill::cast()
{
    currentCooldownSec = cooldownMs / 1000.0f;
}

void Skill::updateCooldown(float deltaSeconds)
{
    if (currentCooldownSec > 0.0f) {
        currentCooldownSec -= deltaSeconds;
        if (currentCooldownSec < 0.0f) currentCooldownSec = 0.0f;
    }
}
