#include "AnimationPlayer.h"

AnimationPlayer::AnimationPlayer() {}

void AnimationPlayer::play(const AnimationClip* c, bool l)
{
    if (!c || !c->isValid()) return;
    clip = c;
    loop = l;
    currentFrame = 0;
    accumulatorMs = 0.0f;
    playing = true;
}

void AnimationPlayer::stop()
{
    playing = false;
}

void AnimationPlayer::update(float deltaSeconds)
{
    if (!playing || !clip) return;

    const auto& frames = clip->frames();
    if (frames.empty()) return;

    accumulatorMs += deltaSeconds * 1000.0f;
    int duration = frames[currentFrame].durationMs;
    while (accumulatorMs >= duration && playing)
    {
        accumulatorMs -= duration;

        int nextFrame = currentFrame + 1;
        if (nextFrame >= (int)frames.size()) {
            if (loop) {
                nextFrame = 0;
            } else {
                currentFrame = (int)frames.size() - 1;
                playing = false;
                break;
            }
        }

        currentFrame = nextFrame;

        if (playing) duration = frames[currentFrame].durationMs;
    }
}

QPixmap AnimationPlayer::getCurrentFrame() const
{
    if (!clip || clip->frames().empty())
        return QPixmap();

    const auto& frames = clip->frames();
    int frameCount = frames.size();

    int safeFrame = currentFrame % frameCount;

    return frames[safeFrame].image;
}

bool AnimationPlayer::isFinished() const
{
    return !playing && !loop && clip && currentFrame == (int)clip->frames().size() - 1;
}
