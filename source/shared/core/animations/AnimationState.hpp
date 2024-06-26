#ifndef __ANIMATION_HPP__
#define __ANIMATION_HPP__

#include "core/animations/AnimationData.hpp"

#include "core/math/Glm.hpp"

#include <vector>
#include <memory>

namespace Soldank
{
class AnimationState
{
public:
    AnimationState(std::shared_ptr<const AnimationData> animation_data);

    void DoAnimation();
    const glm::vec2& GetPosition(unsigned int index) const;
    unsigned int GetFramesCount() const;
    bool IsAny(const std::vector<AnimationType>& animations) const;

    AnimationType GetType() const { return animation_data_->GetAnimationType(); };

    int GetSpeed() const { return speed_; }
    void SetSpeed(int new_speed) { speed_ = new_speed; }

    unsigned int GetFrame() const { return frame_; }
    void SetFrame(unsigned int new_frame) { frame_ = new_frame; }
    void SetNextFrame() { frame_++; }

private:
    std::shared_ptr<const AnimationData> animation_data_;

    int speed_;
    int count_;
    unsigned int frame_;
};
} // namespace Soldank

#endif
