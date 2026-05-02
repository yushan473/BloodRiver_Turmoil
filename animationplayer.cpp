#include "AnimationPlayer.h"

AnimationPlayer::AnimationPlayer() {}

void AnimationPlayer::play(const AnimationClip* clip, bool loop)
{
    if (!clip || !clip->isValid()) return;
    m_clip = clip;
    m_loop = loop;
    m_currentFrame = 0;
    m_accumulatorMs = 0.0f;
    m_playing = true;
}

void AnimationPlayer::stop()
{
    m_playing = false;
}

void AnimationPlayer::update(float deltaSeconds)
{
    if (!m_playing || !m_clip) return;

    const auto& frames = m_clip->frames();
    if (frames.empty()) return;

    m_accumulatorMs += deltaSeconds * 1000.0f;
    int duration = frames[m_currentFrame].durationMs;
    while (m_accumulatorMs >= duration && m_playing)
    {
        m_accumulatorMs -= duration;

        // ==========================================
        // 【修复】先算下一帧，绝不越界！
        // ==========================================
        int nextFrame = m_currentFrame + 1;
        if (nextFrame >= (int)frames.size()) {
            if (m_loop) {
                nextFrame = 0;
            } else {
                m_currentFrame = (int)frames.size() - 1;
                m_playing = false;
                break;
            }
        }

        m_currentFrame = nextFrame;

        if (m_playing) duration = frames[m_currentFrame].durationMs;
    }
}

QPixmap AnimationPlayer::getCurrentFrame() const
{
    // 1. 空检查
    if (!m_clip || m_clip->frames().empty())
        return QPixmap();

    // 2. 【关键修复】强制保证帧下标永远合法！！
    const auto& frames = m_clip->frames();
    int frameCount = frames.size();

    // 永远取模，下标 0 ~ frameCount-1，永不越界，永不返回空
    int safeFrame = m_currentFrame % frameCount;

    // 3. 永远返回有效图片，绝无空帧！
    return frames[safeFrame].image;
}

bool AnimationPlayer::isFinished() const
{
    return !m_playing && !m_loop && m_clip && m_currentFrame == (int)m_clip->frames().size() - 1;
}
