#ifndef __PLAYER_POLL_GROUP_HPP__
#define __PLAYER_POLL_GROUP_HPP__

#include "networking/poll_groups/PollGroupBase.hpp"
#include "networking/types/Connection.hpp"

#include <steam/steamnetworkingsockets.h>

#include <unordered_map>
#include <string>

namespace Soldat
{
class PlayerPollGroup : public PollGroupBase
{
public:
    PlayerPollGroup(ISteamNetworkingSockets* interface);

    void PollIncomingMessages() override;
    void AcceptConnection(SteamNetConnectionStatusChangedCallback_t* new_connection_info) override;
};
} // namespace Soldat

#endif
