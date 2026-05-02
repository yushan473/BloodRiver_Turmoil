#include "Player.h"
#include "animationclip.h"
#include <QPainter>
#include <QDebug>

Player::Player()
{
    m_transform.position = {400, 500};  // 初始位置
    // 添加一个示例技能（实际由外部配置）
    m_skills.emplace_back("普通攻击", 500, 10, nullptr); // 冷却0.5秒，伤害10
}

void Player::setClips(AnimationClip* idle, AnimationClip* walk, AnimationClip* attack)
{
    m_idleClip = idle;
    m_walkClip = walk;
    m_attackClip = attack;
    AnimationClip walkClip;
    // 假设你的角色行走动画有 8 帧，每 100 毫秒切换一帧，速度刚刚好
    walkClip.loadFromSpriteSheet(":/image/player_walk01.png", 8, 100);
    // 默认开始待机动画
    if (m_idleClip) m_animPlayer.play(m_idleClip, true);
}

void Player::update(float deltaSeconds)
{
    // 1. 移动
    float move = 0.0f;
    if (m_leftPressed) move = -1.0f;
    if (m_rightPressed) move = 1.0f;
    m_transform.position.rx() += move * m_speed * deltaSeconds;

    // 边界限制（假设游戏场景宽度800）
    float halfW = 30; // 半宽，可改为从当前动画帧获取
    if (m_transform.position.x() - halfW < 0) m_transform.position.setX(halfW);
    if (m_transform.position.x() + halfW > 800) m_transform.position.setX(800 - halfW);

    // 2. 攻击计时
    if (m_isAttacking) {
        m_attackTimer -= deltaSeconds;
        if (m_attackTimer <= 0.0f) {
            m_isAttacking = false;
            // 攻击结束，恢复移动时的动画（由updateAnimation处理）
        }
    }

    // 3. 更新技能冷却
    for (auto& skill : m_skills) {
        skill.updateCooldown(deltaSeconds);
    }

    // 4. 更新动画（根据当前状态）
    updateAnimation(deltaSeconds);
    m_animPlayer.update(deltaSeconds);
}

void Player::updateAnimation(float deltaSeconds)
{
    // 如果正在攻击，优先播放攻击动画（且不切换）
    if (m_isAttacking) {
        // 已经在攻击动画中，不需要额外动作
        return;
    }

    // 根据移动输入决定待机/走路
    bool moving = (m_leftPressed || m_rightPressed);
    if (moving && m_walkClip && m_animPlayer.isPlaying() && m_animPlayer.getCurrentFrame().isNull()) {
        m_animPlayer.play(m_walkClip, true);
    } else if (!moving && m_idleClip && (m_animPlayer.isPlaying() && m_animPlayer.getCurrentFrame().isNull())) {
        m_animPlayer.play(m_idleClip, true);
    }
}

void Player::attack()
{
    if (m_isAttacking) return;  // 攻击动画未结束
    if (m_skills.empty()) return;
    Skill& skill = m_skills[0];
    if (!skill.canCast()) return;

    // 施放技能（触发冷却）
    skill.cast();

    // 播放攻击动画
    if (m_attackClip) {
        m_animPlayer.play(m_attackClip, false);
        m_isAttacking = true;
        m_attackTimer = 0.3f;   // 假设攻击动画持续0.3秒，之后自动结束
    }
    // 伤害判定实际应由 GameWidget 在攻击瞬间检测与敌人的碰撞，这里我们稍后实现
}

void Player::draw(QPainter* painter) const
{
    QPixmap frame = m_animPlayer.getCurrentFrame();
    if (!frame.isNull()) {
        painter->drawPixmap(m_transform.position.x() - frame.width()/2,
                            m_transform.position.y() - frame.height()/2,
                            frame);
    } else {
        // fallback：绘制一个矩形
        painter->setBrush(Qt::green);
        painter->drawRect(m_transform.position.x() - 20, m_transform.position.y() - 20, 40, 40);
    }
}

QRectF Player::getCollisionRect() const
{
    // 暂时用固定尺寸40x40，可改为从当前帧获取
    return QRectF(m_transform.position.x() - 20, m_transform.position.y() - 20, 40, 40);
}

void Player::takeDamage(int amount)
{
    m_health -= amount;
    if (m_health < 0) m_health = 0;
    qDebug() << "Player health:" << m_health;
}
