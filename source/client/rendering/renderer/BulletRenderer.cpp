#include "BulletRenderer.hpp"

#include "core/types/BulletType.hpp"
#include "core/types/WeaponType.hpp"
#include "rendering/data/sprites/SpriteTypes.hpp"
#include "rendering/shaders/ShaderSources.hpp"
#include "rendering/renderer/Renderer.hpp"

#include "core/math/Calc.hpp"

#include <numbers>
#include <cmath>

namespace Soldank
{

BulletRenderer::BulletRenderer(const Sprites::SpriteManager& sprite_manager)
    : shader_(ShaderSources::DYNAMIC_COLOR_VERTEX_SHADER_SOURCE,
              ShaderSources::DYNAMIC_COLOR_FRAGMENT_SHADER_SOURCE)
{
    LoadSpriteData(sprite_manager, Sprites::WeaponSpriteType::Knife, { 0.5, 0.5 });
    LoadSpriteData(sprite_manager, Sprites::WeaponSpriteType::Knife2, { 0.5, 0.5 });
    LoadSpriteData(sprite_manager, Sprites::WeaponSpriteType::Bullet, { 0.5, 0.5 });
    LoadSpriteData(sprite_manager, Sprites::WeaponSpriteType::DeaglesBullet, { 0.5, 0.5 });
}

void BulletRenderer::LoadSpriteData(const Sprites::SpriteManager& sprite_manager,
                                    Sprites::WeaponSpriteType weapon_sprite_type,
                                    glm::vec2 pivot)
{
    auto texture_data = sprite_manager.GetBulletTexture(weapon_sprite_type);

    pivot.x *= (float)texture_data.width;
    pivot.y *= (float)texture_data.height;
    float w0 = 0.0F - pivot.x;
    float w1 = (float)texture_data.width - pivot.x;
    float h0 = 0.0F - pivot.y;
    float h1 = (float)texture_data.height - pivot.y;

    // clang-format off
    std::vector<float> vertices{
      // position     // texture
      w0, h0, 1.0F,   0.0F, 0.0F,
      w1, h0, 1.0F,   1.0F, 0.0F,
      w0, h1, 1.0F,   0.0F, 1.0F,
      w1, h1, 1.0F,   1.0F, 1.0F
    };
    // clang-format on

    std::vector<unsigned int> indices{ 0, 1, 2, 1, 3, 2 };

    unsigned int vbo = Renderer::CreateVBO(vertices, GL_STATIC_DRAW);
    unsigned int ebo = Renderer::CreateEBO(indices, GL_STATIC_DRAW);
    weapon_sprite_type_to_gl_data_[weapon_sprite_type] = { vbo, ebo, texture_data };
}

void BulletRenderer::Render(glm::mat4 transform, const Bullet& bullet, double frame_percent)
{
    if (!bullet.active) {
        frame_percent = 1.0F;
    }

    auto pos =
      Calc::Lerp(bullet.particle.old_position, bullet.particle.position, (float)frame_percent);

    float rot = 0.0F;
    glm::vec2 scale;
    float alpha = 255.0F;
    BulletSpriteData bullet_sprite_data{};

    switch (bullet.style) {
        case BulletType::ThrownKnife: {
            auto t = Calc::Lerp(bullet.timeout_prev, bullet.timeout, (float)frame_percent);
            rot = 0.0F;
            scale = { 1.0F, 1.0F };

            if (bullet.particle.velocity_.x >= 0.0F) {
                rot = t / std::numbers::pi;
                bullet_sprite_data =
                  weapon_sprite_type_to_gl_data_.at(Sprites::WeaponSpriteType::Knife);
            } else {
                rot = -(t / std::numbers::pi);
                // auto bullet_type = WeaponType::Knife2;
                bullet_sprite_data =
                  weapon_sprite_type_to_gl_data_.at(Sprites::WeaponSpriteType::Knife2);
            }
            break;
        }
        default: {
            auto hit =
              Calc::Lerp(bullet.hit_multiply_prev, bullet.hit_multiply, (float)frame_percent);

            bullet_sprite_data =
              weapon_sprite_type_to_gl_data_.at(Sprites::WeaponSpriteType::Bullet);

            scale = { Calc::Vec2Length(bullet.particle.velocity_) / 13.0F, 1.0F };
            auto dist = Calc::Vec2Length(pos - bullet.initial_position);

            if (dist < scale.x * (float)bullet_sprite_data.texture_data.width) {
                scale.x = dist / (scale.x * (float)bullet_sprite_data.texture_data.width);
            }
            alpha = std::max(50.0F, std::min(230.0F, 255.0F * hit * (scale.x * scale.x) / 4.63F));

            rot = Calc::Vec2Angle(-bullet.particle.velocity_);
        }
    }

    if (bullet.style == BulletType::Bullet && bullet.weapon == WeaponType::DesertEagles) {
        bullet_sprite_data =
          weapon_sprite_type_to_gl_data_.at(Sprites::WeaponSpriteType::DeaglesBullet);
    }

    shader_.Use();
    Renderer::SetupVertexArray(bullet_sprite_data.vbo, bullet_sprite_data.ebo, false, true);

    // TODO: magic number, this is in mod.ini
    scale /= 4.5F;

    glm::mat4 current_scenery_transform = transform;

    current_scenery_transform =
      glm::translate(current_scenery_transform, glm::vec3(pos.x, -pos.y, 0.0));
    current_scenery_transform =
      glm::rotate(current_scenery_transform, rot, glm::vec3(0.0, 0.0, 1.0));
    current_scenery_transform =
      glm::scale(current_scenery_transform, glm::vec3(scale.x, scale.y, 0.0));

    shader_.SetMatrix4("transform", current_scenery_transform);
    shader_.SetVec4("color", glm::vec4(1.0F, 1.0F, 1.0F, alpha / 255.0F));

    Renderer::BindTexture(bullet_sprite_data.texture_data.opengl_id);
    Renderer::DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
} // namespace Soldank
