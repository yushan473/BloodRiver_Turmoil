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
    bool isPlaying() const { return m_playing; }
    bool isFinished() const;   // 非循环动画是否播放完毕

private:
    const AnimationClip* m_clip = nullptr;
    int m_currentFrame = 0;
    float m_accumulatorMs = 0.0f;
    bool m_playing = false;
    bool m_loop = false;
};

#endif // ANIMATIONPLAYER_H
