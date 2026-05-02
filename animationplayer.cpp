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
        m_currentFrame++;
        if (m_currentFrame >= (int)frames.size())
        {
            if (m_loop) {
                m_currentFrame = 0;
            } else {
                m_currentFrame = (int)frames.size() - 1;
                m_playing = false;
                break;
            }
        }
        if (m_playing) duration = frames[m_currentFrame].durationMs;
    }
}

QPixmap AnimationPlayer::getCurrentFrame() const
{
    if (!m_clip || m_clip->frames().empty()) return QPixmap();
    return m_clip->frames()[m_currentFrame].image;
}

bool AnimationPlayer::isFinished() const
{
    return !m_playing && !m_loop && m_clip && m_currentFrame == (int)m_clip->frames().size() - 1;
}
