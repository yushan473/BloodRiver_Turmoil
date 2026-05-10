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

    void loadFromSpriteSheet(const QString& path, int frameCount, int frameDurationMs);

    void addFrame(const QPixmap& pix, int durationMs);

    const std::vector<Frame>& frames() const { return framesList; }
    bool isValid() const { return !framesList.empty(); }

private:
    std::vector<Frame> framesList;
};

#endif // ANIMATIONCLIP_H
