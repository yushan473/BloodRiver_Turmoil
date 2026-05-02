#include "animationclip.h"

AnimationClip::AnimationClip() {}

void AnimationClip::loadFromSpriteSheet(const QString& path, int frameCount, int frameDurationMs)
{
    QPixmap sheet(path);
    if (sheet.isNull()) return;
    int frameWidth = sheet.width() / frameCount;
    int frameHeight = sheet.height();
    for (int i = 0; i < frameCount; ++i) {
        QPixmap frame = sheet.copy(i * frameWidth, 0, frameWidth, frameHeight);
        addFrame(frame, frameDurationMs);
    }
}

void AnimationClip::addFrame(const QPixmap& pix, int durationMs)
{
    m_frames.push_back({pix, durationMs});
}
