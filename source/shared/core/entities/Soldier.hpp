#ifndef __SOLDIER_HPP__
#define __SOLDIER_HPP__

#include "core/animations/Animation.hpp"
#include "core/physics/Particles.hpp"
#include "core/state/Control.hpp"
#include "core/entities/Weapon.hpp"
#include "core/entities/Bullet.hpp"
#include "core/math/Glm.hpp"

#include <cstdint>
#include <vector>
#include <memory>
#include <array>

namespace Soldank
{
struct Soldier
{
    Soldier(std::uint8_t soldier_id, glm::vec2 spawn_position);

    std::uint8_t id;

    glm::vec2 camera;
    glm::vec2 camera_prev;
    glm::vec2 mouse;
    float game_width;
    float game_height;

    bool active;
    bool dead_meat;
    std::uint8_t style;
    std::uint32_t num;
    std::uint8_t visible;
    bool on_ground;
    bool on_ground_for_law;
    bool on_ground_last_frame;
    bool on_ground_permanent;
    std::int8_t direction;
    std::int8_t old_direction;
    float health;
    std::uint8_t alpha;
    std::int32_t jets_count;
    std::int32_t jets_count_prev;
    std::uint8_t wear_helmet;
    std::uint8_t has_cigar;
    float vest;
    std::int32_t idle_time;
    std::int8_t idle_random;
    std::uint8_t stance;
    std::uint8_t on_fire;
    std::uint8_t collider_distance;
    bool half_dead;
    std::shared_ptr<ParticleSystem> skeleton;
    AnimationState legs_animation;
    AnimationState body_animation;
    Control control;
    std::uint8_t active_weapon;
    std::vector<Weapon> weapons;
    std::array<WeaponType, 2> weapon_choices;
    std::uint8_t fired;
    Particle particle;
    bool is_shooting;

    int ticks_to_respawn;
};
} // namespace Soldank

#endif
