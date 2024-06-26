#include "networking/event_handlers/PlayerLeaveNetworkEventHandler.hpp"

#include "spdlog/spdlog.h"

#include <chrono>

namespace Soldank
{
PlayerLeaveNetworkEventHandler::PlayerLeaveNetworkEventHandler(const std::shared_ptr<IWorld>& world)
    : world_(world)
{
}

NetworkEventHandlerResult PlayerLeaveNetworkEventHandler::HandleNetworkMessageImpl(
  unsigned int /*sender_connection_id*/,
  std::uint8_t soldier_id)
{
    auto& state = world_->GetStateManager()->GetState();
    for (auto it = state.soldiers.begin(); it != state.soldiers.end();) {
        if (it->id == soldier_id) {
            it = state.soldiers.erase(it);
        } else {
            it++;
        }
    }

    return NetworkEventHandlerResult::Success;
}
} // namespace Soldank
