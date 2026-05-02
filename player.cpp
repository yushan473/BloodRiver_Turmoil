#include "Player.h"
#include "animationclip.h"
#include <QPainter>
#include <QDebug>

Player::Player()
{
    m_transform.position = {50, 200};  // 初始位置：左侧靠下
    // 添加一个示例技能（实际由外部配置）
    m_skills.emplace_back("普通攻击", 500, 10, nullptr); // 冷却0.5秒，伤害10
}

void Player::setClips(AnimationClip* idle, AnimationClip* walk, AnimationClip* attack)
{
    m_idleClip = idle;
    m_walkClip = walk;
    m_attackClip = attack;

    if (m_walkClip) {
        m_walkClip->loadFromSpriteSheet(":/res/image/player_walk01.png", 8, 100);
    }

    if (m_idleClip) {
        m_idleClip->loadFromSpriteSheet(":/res/image/p_idle.png", 4, 150);
        m_animPlayer.play(m_idleClip, true);
    }
}

void Player::update(float deltaSeconds)
{
    float moveX = 0.0f;
    float moveY = 0.0f;
    if (m_leftPressed) moveX = -1.0f;
    if (m_rightPressed) moveX = 1.0f;
    if (m_upPressed) moveY = -1.0f;
    if (m_downPressed) moveY = 1.0f;
    m_transform.position.rx() += moveX * m_speed * deltaSeconds;
    m_transform.position.ry() += moveY * m_speed * deltaSeconds;

    float halfW = 24;
    float halfH = 24;
    if (m_transform.position.x() - halfW < 0) m_transform.position.setX(halfW);
    if (m_transform.position.x() + halfW > 256) m_transform.position.setX(256 - halfW);
    if (m_transform.position.y() - halfH < 0) m_transform.position.setY(halfH);
    if (m_transform.position.y() + halfH > 256) m_transform.position.setY(256 - halfH);

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
    Q_UNUSED(deltaSeconds);
    if (m_isAttacking) return;

    bool moving = (m_leftPressed || m_rightPressed || m_upPressed || m_downPressed);
    if (moving) {
        if (m_walkClip && (!m_animPlayer.isPlaying() || m_animPlayer.isFinished())) {
            m_animPlayer.play(m_walkClip, true);
        }
    } else {
        if (m_idleClip && (!m_animPlayer.isPlaying() || m_animPlayer.isFinished())) {
            m_animPlayer.play(m_idleClip, true);
        }
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
        int x = m_transform.position.x() - 24;
        int y = m_transform.position.y() - 24;
        painter->drawPixmap(x, y, frame);
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
