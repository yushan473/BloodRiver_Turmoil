#ifndef ANIMATIONCLIP_H
#define ANIMATIONCLIP_H

#include<QPixmap>
#include<QString>
#include<vector>

struct Frame{
    QPixmap image;
    int durationMs;
};

class AnimationClip
{
public:
    AnimationClip();

    // 从水平精灵表加载（每帧等宽）
    void loadFromSpriteSheet(const QString& path, int frameCount, int frameDurationMs);

    // 手动添加一帧（用于测试or程序生成）
    void addFrame(const QPixmap& pix, int durationMs);

    // 获取所有帧
    const std::vector<Frame>& frames() const { return m_frames; }
    bool isValid() const { return !m_frames.empty(); }

private:
    std::vector<Frame> m_frames;


};

#endif // ANIMATIONCLIP_H
