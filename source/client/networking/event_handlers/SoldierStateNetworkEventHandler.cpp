#include "networking/event_handlers/SoldierStateNetworkEventHandler.hpp"

#include "communication/NetworkPackets.hpp"

#include "core/physics/SoldierSkeletonPhysics.hpp"

#include "spdlog/spdlog.h"

namespace Soldat
{
SoldierStateNetworkEventHandler::SoldierStateNetworkEventHandler(
  const std::shared_ptr<IWorld>& world,
  const std::shared_ptr<ClientState>& client_state)
    : world_(world)
    , client_state_(client_state)
{
}

NetworkEventHandlerResult SoldierStateNetworkEventHandler::HandleNetworkMessageImpl(
  SoldierStatePacket soldier_state_packet)
{
    unsigned int soldier_id = soldier_state_packet.player_id;
    glm::vec2 soldier_position = { soldier_state_packet.position_x,
                                   soldier_state_packet.position_y };
    glm::vec2 soldier_old_position = { soldier_state_packet.old_position_x,
                                       soldier_state_packet.old_position_y };
    AnimationType body_animation_type = soldier_state_packet.body_animation_type;
    unsigned int body_animation_frame = soldier_state_packet.body_animation_frame;
    int body_animation_speed = soldier_state_packet.body_animation_speed;
    AnimationType legs_animation_type = soldier_state_packet.legs_animation_type;
    unsigned int legs_animation_frame = soldier_state_packet.legs_animation_frame;
    int legs_animation_speed = soldier_state_packet.legs_animation_speed;
    glm::vec2 soldier_velocity = { soldier_state_packet.velocity_x,
                                   soldier_state_packet.velocity_y };
    glm::vec2 soldier_force = { soldier_state_packet.force_x, soldier_state_packet.force_y };
    bool on_ground = soldier_state_packet.on_ground;
    bool on_ground_for_law = soldier_state_packet.on_ground_for_law;
    bool on_ground_last_frame = soldier_state_packet.on_ground_last_frame;
    bool on_ground_permanent = soldier_state_packet.on_ground_permanent;
    std::uint8_t stance = soldier_state_packet.stance;
    float mouse_position_x = soldier_state_packet.mouse_position_x;
    float mouse_position_y = soldier_state_packet.mouse_position_y;
    bool using_jets = soldier_state_packet.using_jets;
    std::int32_t jets_count = soldier_state_packet.jets_count;
    unsigned int active_weapon = soldier_state_packet.active_weapon;
    unsigned int last_processed_input_id = soldier_state_packet.last_processed_input_id;

    bool is_soldier_id_me = false;
    if (client_state_->client_soldier_id.has_value()) {
        is_soldier_id_me = *client_state_->client_soldier_id == soldier_id;
    }
    if (is_soldier_id_me) {
        client_state_->soldier_position_server_pov = { soldier_position.x, soldier_position.y };
    }
    const auto& state = world_->GetState();

    for (auto& soldier : state->soldiers) {
        if (soldier.id == soldier_id) {
            soldier.particle.old_position = soldier_old_position;
            soldier.particle.position = soldier_position;
            soldier.body_animation = AnimationState(body_animation_type);
            soldier.body_animation.SetFrame(body_animation_frame);
            soldier.body_animation.SetSpeed(body_animation_speed);

            soldier.legs_animation = AnimationState(legs_animation_type);
            soldier.legs_animation.SetFrame(legs_animation_frame);
            soldier.legs_animation.SetSpeed(legs_animation_speed);

            soldier.particle.SetVelocity(soldier_velocity);
            soldier.particle.SetForce(soldier_force);

            soldier.on_ground = on_ground;
            soldier.on_ground_for_law = on_ground_for_law;
            soldier.on_ground_last_frame = on_ground_last_frame;
            soldier.on_ground_permanent = on_ground_permanent;

            soldier.stance = stance;

            // TODO: make mouse position not game width and game height dependent
            soldier.game_width = 640.0;
            soldier.game_height = 480.0;
            soldier.mouse.x = mouse_position_x;
            soldier.mouse.y = 480.0F - mouse_position_y;

            soldier.camera.x =
              soldier.particle.position.x + (float)(soldier.mouse.x - (soldier.game_width / 2));
            soldier.camera.y = soldier.particle.position.y -
                               (float)((480.0F - soldier.mouse.y) - (soldier.game_height / 2));

            // TODO: server should send mouse_aim and here we should recalculate the rest
            soldier.control.mouse_aim_x =
              (soldier.mouse.x - (float)soldier.game_width / 2.0F + soldier.camera.x);
            soldier.control.mouse_aim_y =
              (soldier.mouse.y - (float)soldier.game_height / 2.0F + soldier.camera.y);

            if ((float)soldier.control.mouse_aim_x >= soldier.particle.position.x) {
                soldier.direction = 1;
            } else {
                soldier.direction = -1;
            }

            // TODO: there is a visual bug with feet when another soldier is using jets and going
            // backwards
            soldier.control.jets = using_jets;
            soldier.jets_count = jets_count;

            soldier.active_weapon = active_weapon;

            if (!is_soldier_id_me || !client_state_->client_side_prediction) {
                RepositionSoldierSkeletonParts(soldier);

                if (!soldier.dead_meat) {
                    soldier.body_animation.DoAnimation();
                    soldier.legs_animation.DoAnimation();
                    soldier.skeleton->DoVerletTimestepFor(22, 29);
                    soldier.skeleton->DoVerletTimestepFor(24, 30);
                }

                if (soldier.dead_meat) {
                    soldier.skeleton->DoVerletTimestep();
                    soldier.particle.position = soldier.skeleton->GetPos(12);
                    // CheckSkeletonOutOfBounds;
                }
            }
            // spdlog::info(
            //   "{}, Soldier {} pos: {}, {}; old_pod: {}, {}; velocity: {}, {}; force: {}, {}",
            //   last_processed_input_id,
            //   soldier_id,
            //   soldier_position.x,
            //   soldier_position.y,
            //   soldier_old_position.x,
            //   soldier_old_position.y,
            //   soldier_velocity.x,
            //   soldier_velocity.y,
            //   soldier_force.x,
            //   soldier_force.y);
        }
    }

    if (client_state_->server_reconciliation && is_soldier_id_me) {
        for (auto it = client_state_->pending_inputs.begin();
             it != client_state_->pending_inputs.end();) {
            if (it->input_sequence_id <= last_processed_input_id) {
                it = client_state_->pending_inputs.erase(it);
            } else {
                const auto& player_control = it->control;
                world_->UpdateRightButtonState(soldier_id, player_control.right);
                world_->UpdateLeftButtonState(soldier_id, player_control.left);
                world_->UpdateJumpButtonState(soldier_id, player_control.up);
                world_->UpdateCrouchButtonState(soldier_id, player_control.down);
                world_->UpdateProneButtonState(soldier_id, player_control.prone);
                world_->UpdateChangeButtonState(soldier_id, player_control.change);
                world_->UpdateThrowGrenadeButtonState(soldier_id, player_control.throw_grenade);
                world_->UpdateDropButtonState(soldier_id, player_control.drop);

                world_->UpdateMousePosition(soldier_id,
                                            { it->mouse_position_x, it->mouse_position_y });
                world_->UpdateFireButtonState(soldier_id, player_control.fire); // TODO: is
                // it needed?
                world_->UpdateJetsButtonState(soldier_id, player_control.jets);
                world_->UpdateSoldier(soldier_id);
                // spdlog::info(
                //   "{}, Soldier {} pos: {}, {}; old_pos: {}, {}; velocity: {}, {}; force: {},
                //   {}", it->input_sequence_id, soldier_id,
                //   world_->GetSoldier(soldier_id).particle.position.x,
                //   world_->GetSoldier(soldier_id).particle.position.y,
                //   world_->GetSoldier(soldier_id).particle.old_position.x,
                //   world_->GetSoldier(soldier_id).particle.old_position.y,
                //   world_->GetSoldier(soldier_id).particle.GetVelocity().x,
                //   world_->GetSoldier(soldier_id).particle.GetVelocity().y,
                //   world_->GetSoldier(soldier_id).particle.GetForce().x,
                //   world_->GetSoldier(soldier_id).particle.GetForce().y);
                it++;
            }
        }
    }

    return NetworkEventHandlerResult::Success;
}
} // namespace Soldat
