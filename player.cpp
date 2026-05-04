#include "Player.h"
#include "animationclip.h"
#include <QPainter>
#include <QDebug>

Player::Player()
{
    m_transform.position = {100, 396};
    m_skills.emplace_back("防守技能", 0, 5, nullptr);
    m_skills.emplace_back("攻击技能", 0, 10, nullptr);
}

void Player::setClips(AnimationClip* idle, AnimationClip* walk, AnimationClip* attack1, AnimationClip* attack2)
{
    m_idleClip = idle;
    m_walkClip = walk;
    m_attackLv1Clip = attack1;
    m_attackLv2Clip = attack2;

    if (m_walkClip) {
        m_walkClip->loadFromSpriteSheet(":/res/image/player_walk01.png", 8, 100);
    }

    if (m_idleClip) {
        m_idleClip->loadFromSpriteSheet(":/res/image/p_idle.png", 3, 150);
        m_animPlayer.play(m_idleClip, true);
    }

    if (m_attackLv1Clip) {
        m_attackLv1Clip->loadFromSpriteSheet(":/res/image/player_attack_lv1.png", 14, 70);
    }

    if (m_attackLv2Clip) {
        m_attackLv2Clip->loadFromSpriteSheet(":/res/image/player_attack_lv2.png", 13, 77);
    }
}

void Player::update(float deltaSeconds)
{
    float moveX = 0.0f;
    if (m_leftPressed) moveX = -1.0f;
    if (m_rightPressed) moveX = 1.0f;
    m_transform.position.rx() += moveX * m_speed * deltaSeconds;

    float halfW = 24;
    if (m_transform.position.x() < halfW) m_transform.position.setX(halfW);
    if (m_transform.position.x() > 512 - halfW) m_transform.position.setX(512 - halfW);

    if (m_jumpPressed && m_isOnGround) {
        m_velocityY = m_jumpForce;
        m_isOnGround = false;
        m_jumpPressed = false;
    }

    if (!m_isOnGround) {
        m_velocityY += m_gravity * deltaSeconds;
        m_transform.position.ry() += m_velocityY * deltaSeconds;

        const float groundY = 396.0f;  // 512画布下的地面位置 (原256画布的192 × 2 + 12)
        if (m_transform.position.y() >= groundY) {
            m_transform.position.setY(groundY);
            m_velocityY = 0.0f;
            m_isOnGround = true;
        }
    }

    if (m_isAttacking) {
        m_attackTimer -= deltaSeconds;
        if (m_attackTimer <= 0.0f) {
            m_isAttacking = false;
        }
    }

    for (auto& skill : m_skills) {
        skill.updateCooldown(deltaSeconds);
    }

    updateAnimation(deltaSeconds);
    m_animPlayer.update(deltaSeconds);
}

void Player::updateAnimation(float deltaSeconds)
{
    Q_UNUSED(deltaSeconds);
    if (m_isAttacking) return;

    bool moving = (m_leftPressed || m_rightPressed);

    if (moving) {
        if (m_walkClip && m_animPlayer.getCurrentClip() != m_walkClip) {
            m_animPlayer.play(m_walkClip, true);
        }
    } else {
        if (m_idleClip && m_animPlayer.getCurrentClip() != m_idleClip) {
            m_animPlayer.play(m_idleClip, true);
        }
    }
}

void Player::attack(int skillIndex)
{
    if (m_isAttacking) return;
    if (skillIndex < 0 || skillIndex >= (int)m_skills.size()) return;

    m_currentAttackSkill = skillIndex;
    Skill& skill = m_skills[skillIndex];

    skill.cast();

    AnimationClip* clip = (skillIndex == 0) ? m_attackLv1Clip : m_attackLv2Clip;
    if (clip) {
        m_animPlayer.play(clip, false);
        m_isAttacking = true;
        m_attackTimer = 0.5f;
    }
}

void Player::draw(QPainter* painter) const
{
    QPixmap frame = m_animPlayer.getCurrentFrame();

    if (!frame.isNull()) {
        int x = m_transform.position.x() - frame.width() / 2;
        int y = m_transform.position.y() - frame.height() + 12;
        painter->drawPixmap(x, y, frame);

        float healthPercent = (float)m_health / m_maxHealth;
        int barWidth = 32;
        int barHeight = 4;
        int barX = m_transform.position.x() - barWidth / 2;
        int barY = m_transform.position.y() - frame.height() + 12 - 8;

        painter->setPen(QColor(136, 136, 136));
        painter->setBrush(QColor(58, 58, 58));
        painter->drawRect(barX, barY, barWidth, barHeight);
        painter->setBrush(QColor(126, 140, 132));
        painter->drawRect(barX, barY, barWidth * healthPercent, barHeight);
    }
}

QRectF Player::getCollisionRect() const
{
    QPixmap frame = m_animPlayer.getCurrentFrame();
    int w = frame.width() > 0 ? frame.width() : 40;
    int h = frame.height() > 0 ? frame.height() : 40;
    return QRectF(m_transform.position.x() - w/2, m_transform.position.y() - h, w, h);
}

QRectF Player::getAttackRange() const
{
    // 扩大攻击范围，特别是向上的范围，以打到浮空的慕阴真
    return QRectF(m_transform.position.x() - 50, m_transform.position.y() - 120, 100, 120);
}

void Player::takeDamage(int amount)
{
    m_health -= amount;
    if (m_health < 0) m_health = 0;
    qDebug() << "Player health:" << m_health;
}
