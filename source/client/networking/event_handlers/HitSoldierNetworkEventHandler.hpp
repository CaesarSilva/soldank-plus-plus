#ifndef __HIT_SOLDIER_NETWORK_EVENT_HANDLER_HPP__
#define __HIT_SOLDIER_NETWORK_EVENT_HANDLER_HPP__

#include "core/IWorld.hpp"

#include "communication/NetworkEventDispatcher.hpp"

namespace Soldank
{
class HitSoldierNetworkEventHandler : public NetworkEventHandlerBase<std::uint8_t, float>
{
public:
    HitSoldierNetworkEventHandler(const std::shared_ptr<IWorld>& world);

private:
    NetworkEvent GetTargetNetworkEvent() const override { return NetworkEvent::HitSoldier; }

    NetworkEventHandlerResult HandleNetworkMessageImpl(unsigned int sender_connection_id,
                                                       std::uint8_t soldier_id,
                                                       float damage) override;

    std::shared_ptr<IWorld> world_;
};
} // namespace Soldank

#endif
