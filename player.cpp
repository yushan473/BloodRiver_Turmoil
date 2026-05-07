#include "Player.h"
#include "animationclip.h"
#include <QPainter>
#include <QDebug>
const float GRAVITY = 700.0f;
const float JUMP_FORCE = -320.0f;

Player::Player()
{
    transform.position = {100, 396};
    skills.emplace_back("防守技能", 500, 5);
    skills.emplace_back("攻击技能", 800, 10);
}

void Player::setClips(AnimationClip* idle, AnimationClip* walk, AnimationClip* attack1, AnimationClip* attack2, AnimationClip* jump)
{
    idleClip = idle;
    walkClip = walk;
    attackLv1Clip = attack1;
    attackLv2Clip = attack2;
    jumpClip = jump;
}

void Player::update(float deltaSeconds)
{
    float moveX = 0.0f;
    if (leftPressed) moveX = -1.0f;
    if (rightPressed) moveX = 1.0f;
    transform.position.rx() += moveX * speed * deltaSeconds;

    //float halfW = 24;
    // if (transform.position.x() < halfW) transform.position.setX(halfW);
    // if (transform.position.x() > 512 - halfW) transform.position.setX(512 - halfW);

    if (jumpPressed) {
        if (onGround) {
            // 地面跳跃
            velocityY = JUMP_FORCE;
            onGround = false;
            jumpPressed = false;
            doubleJumpUsed = false;// 重置二段跳标记
            canDoubleJump = true;// 允许二段跳
        }
        else if (canDoubleJump && !doubleJumpUsed) {
            // 二段跳
            velocityY = JUMP_FORCE;
            doubleJumpUsed = true;
            jumpPressed = false;
        }
    }

    if (!onGround) {
        velocityY += GRAVITY * deltaSeconds;
        transform.position.ry() += velocityY * deltaSeconds;

        const float groundY = 396.0f;
        if (transform.position.y() >= groundY) {
            transform.position.setY(groundY);
            velocityY = 0.0f;
            onGround = true;
            doubleJumpUsed = false;
            canDoubleJump = true;
        }
    }

    if (isAttacking) {
        attackTimer -= deltaSeconds;
        if (attackTimer <= 0.0f) {
            isAttacking = false;
        }
    }

    for (auto& skill : skills) {
        skill.updateCooldown(deltaSeconds);
    }

    //动画（大招时不允许切换）
    if (!isAttacking && !isPlayingSuper) {
        if (!onGround) {
            if (jumpClip && animPlayer.getCurrentClip() != jumpClip) {
                animPlayer.play(jumpClip, true);
            }
        } else {
            bool moving = (leftPressed || rightPressed);
            if (moving) {
                if (walkClip && animPlayer.getCurrentClip() != walkClip) {
                    animPlayer.play(walkClip, true);
                }
            } else {
                if (idleClip && animPlayer.getCurrentClip() != idleClip) {
                    animPlayer.play(idleClip, true);
                }
            }
        }
    }
    animPlayer.update(deltaSeconds);

    if (isPlayingSuper && animPlayer.isFinished()) {
        isPlayingSuper = false;
    }
}


void Player::attack(int skillIndex)
{
    if (isAttacking) return;
    if (skillIndex < 0 || skillIndex >= (int)skills.size()) return;

    Skill& skill = skills[skillIndex];

    if (!skill.canCast()) return;
    currentAttackSkill = skillIndex;
    skill.cast();

    AnimationClip* clip = (skillIndex == 0) ? attackLv1Clip : attackLv2Clip;
    if (clip) {
        animPlayer.play(clip, false);
        isAttacking = true;
        attackTimer = 0.5f;
    }
}

void Player::draw(QPainter* painter) const
{
    QPixmap frame = animPlayer.getCurrentFrame();

    if (!frame.isNull()) {
        int x, y;

        if (isPlayingSuper) {
            // 大招动画：强制居中绘制，让玩家在大招动画中间
            x = transform.position.x() - frame.width() / 2;
            y = transform.position.y() - frame.height() / 2;
        } else {
            // 普通动画：原来的绘制逻辑
            x = transform.position.x() - frame.width() / 2;
            y = transform.position.y() - frame.height() + 12;
        }


        painter->drawPixmap(x, y, frame);

        float healthPercent = (float)health / maxHealth;
        int barWidth = 32;
        int barHeight = 4;
        int barX = transform.position.x() - barWidth / 2;
        int barY = transform.position.y() - frame.height() + 12 - 8;

        painter->setPen(QColor(136, 136, 136));
        painter->setBrush(QColor(58, 58, 58));
        painter->drawRect(barX, barY, barWidth, barHeight);
        painter->setBrush(QColor(126, 140, 132));
        painter->drawRect(barX, barY, barWidth * healthPercent, barHeight);
    }
}

QRectF Player::getCollisionRect() const
{
    QPixmap frame = animPlayer.getCurrentFrame();
    int w = frame.width() > 0 ? frame.width() : 40;
    int h = frame.height() > 0 ? frame.height() : 40;
    return QRectF(transform.position.x() - w/2, transform.position.y() - h, w, h);
}

QRectF Player::getAttackRange() const
{
    return QRectF(transform.position.x() - 50, transform.position.y() - 120, 100, 120);
}

void Player::takeDamage(int amount)
{
    health -= amount;
    if (health < 0) health = 0;
    if (health > maxHealth) health = maxHealth;
    qDebug() << "Player health:" << health;
}

void Player::playSuperAnimation()
{
    qDebug() << "playSuperAnimation 被调用";
    if (superClip) {
        qDebug() << "superClip 不为空，开始播放";
        animPlayer.play(superClip, false);
        isPlayingSuper = true;
    } else {
        qDebug() << "superClip 为空！";
    }
}
