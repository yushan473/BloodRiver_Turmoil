#include "Skill.h"

void Skill::cast()
{
    // 实际使用时，这里会触发伤害逻辑，暂时不做具体实现
    // 可以在外部处理伤害，这里只负责冷却重置
    m_currentCooldownSec = cooldownMs / 1000.0f;
}

void Skill::updateCooldown(float deltaSeconds)
{
    if (m_currentCooldownSec > 0.0f) {
        m_currentCooldownSec -= deltaSeconds;
        if (m_currentCooldownSec < 0.0f) m_currentCooldownSec = 0.0f;
    }
}
