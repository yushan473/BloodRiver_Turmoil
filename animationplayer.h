#ifndef ANIMATIONPLAYER_H
#define ANIMATIONPLAYER_H

#include "AnimationClip.h"

class AnimationPlayer
{
public:
    AnimationPlayer();

    // 播放指定的剪辑，loop是否循环
    void play(const AnimationClip* clip, bool loop = false);
    void stop();
    void update(float deltaSeconds);   // 每帧调用
    QPixmap getCurrentFrame() const;
    bool isPlaying() const { return playing; }
    bool isFinished() const;
    const AnimationClip* getCurrentClip() const { return clip; }

private:
    const AnimationClip* clip = nullptr;
    int currentFrame = 0;
    float accumulatorMs = 0.0f;
    bool playing = false;
    bool loop = false;
};

#endif // ANIMATIONPLAYER_H
