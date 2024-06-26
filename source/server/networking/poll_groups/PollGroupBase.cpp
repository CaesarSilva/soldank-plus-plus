#include "networking/poll_groups/PollGroupBase.hpp"

#include "spdlog/spdlog.h"

#include <cassert>
#include <utility>

namespace Soldank
{
PollGroupBase::PollGroupBase(ISteamNetworkingSockets* interface)
    : IPollGroup(interface)
    , poll_group_handle_(interface->CreatePollGroup())
{
    if (poll_group_handle_ == k_HSteamNetPollGroup_Invalid) {
        spdlog::error("Failed to create a player poll group");
    }
}

PollGroupBase::~PollGroupBase()
{
    GetInterface()->DestroyPollGroup(poll_group_handle_);
    poll_group_handle_ = k_HSteamNetPollGroup_Invalid;
}

void PollGroupBase::CloseConnection(SteamNetConnectionStatusChangedCallback_t* connection_info)
{
    // Ignore if they were not previously connected. (If they disconnected
    // before we accepted the connection.)
    if (connection_info->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
        auto it_client = connections_.find(connection_info->m_hConn);
        assert(it_client != connections_.end());

        std::string debug_log_action;
        if (connection_info->m_info.m_eState ==
            k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
            debug_log_action = "problem detected locally";
        } else {
            debug_log_action = "closed by peer";
        }

        spdlog::info("Connection {} {}, reason {}: {}",
                     connection_info->m_info.m_szConnectionDescription,
                     debug_log_action,
                     connection_info->m_info.m_eEndReason,
                     connection_info->m_info.m_szEndDebug);

        SendStringToAllClients(std::format("{} {}", it_client->second.nick, "has left the server"),
                               it_client->first);
        connections_.erase(it_client);
    } else {
        assert(connection_info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
    }

    auto it_client = FindConnection(connection_info->m_hConn);

    GetInterface()->CloseConnection(connection_info->m_hConn, 0, nullptr, false);

    if (it_client != connections_.end()) {
        EraseConnection(it_client);
    }
}

bool PollGroupBase::AssignConnection(const Connection& connection)
{
    if (!GetInterface()->SetConnectionPollGroup(connection.connection_handle, poll_group_handle_)) {
        GetInterface()->CloseConnection(connection.connection_handle, 0, nullptr, false);
        spdlog::warn("Failed to set poll group?");
        return false;
    }
    connections_[connection.connection_handle] = connection;
    OnAssignConnection(connections_[connection.connection_handle]);

    return true;
}

void PollGroupBase::SendNetworkMessage(HSteamNetConnection connection_id,
                                       const NetworkMessage& network_message)
{
    // TODO: handle result
    auto result = GetInterface()->SendMessageToConnection(connection_id,
                                                          network_message.GetData().data(),
                                                          network_message.GetData().size(),
                                                          k_nSteamNetworkingSend_Unreliable,
                                                          nullptr);
}

void PollGroupBase::SendNetworkMessageToAll(const NetworkMessage& network_message,
                                            std::optional<unsigned int> except_connection_id)
{
    for (auto& connection : connections_) {
        if (except_connection_id.has_value() &&
            *except_connection_id == connection.second.connection_handle) {
            continue;
        }
        // TODO: handle result
        auto result = GetInterface()->SendMessageToConnection(connection.second.connection_handle,
                                                              network_message.GetData().data(),
                                                              network_message.GetData().size(),
                                                              k_nSteamNetworkingSend_Unreliable,
                                                              nullptr);
    }
}

void PollGroupBase::SendReliableNetworkMessage(unsigned int connection_id,
                                               const NetworkMessage& network_message)
{
    // TODO: handle result
    auto result = GetInterface()->SendMessageToConnection(connection_id,
                                                          network_message.GetData().data(),
                                                          network_message.GetData().size(),
                                                          k_nSteamNetworkingSend_Reliable,
                                                          nullptr);
}

void PollGroupBase::SendReliableNetworkMessageToAll(
  const NetworkMessage& network_message,
  std::optional<unsigned int> except_connection_id)
{
    for (auto& connection : connections_) {
        if (except_connection_id.has_value() &&
            *except_connection_id == connection.second.connection_handle) {
            continue;
        }
        // TODO: handle result
        auto result = GetInterface()->SendMessageToConnection(connection.second.connection_handle,
                                                              network_message.GetData().data(),
                                                              network_message.GetData().size(),
                                                              k_nSteamNetworkingSend_Reliable,
                                                              nullptr);
    }
}

void PollGroupBase::OnAssignConnection(Connection& /* connection */) {}

bool PollGroupBase::IsConnectionAssigned(HSteamNetConnection steam_net_connection_handle)
{
    auto it_client = connections_.find(steam_net_connection_handle);
    return it_client != connections_.end();
}

unsigned int PollGroupBase::GetConnectionSoldierId(HSteamNetConnection steam_net_connection_handle)
{
    auto it_client = connections_.find(steam_net_connection_handle);
    return it_client->second.soldier_id;
}

std::string PollGroupBase::GetConnectionSoldierNick(HSteamNetConnection steam_net_connection_handle)
{
    auto it_client = connections_.find(steam_net_connection_handle);
    return it_client->second.nick;
}

HSteamNetPollGroup PollGroupBase::GetPollGroupHandle() const
{
    return poll_group_handle_;
}

std::unordered_map<HSteamNetConnection, Connection>::iterator PollGroupBase::FindConnection(
  HSteamNetConnection steam_net_connection_handle)
{
    return connections_.find(steam_net_connection_handle);
}

HSteamNetConnection PollGroupBase::FindConnectionHandleBySoldierId(unsigned int soldier_id)
{
    for (const auto& connection : connections_) {
        if (connection.second.soldier_id == soldier_id) {
            return connection.second.connection_handle;
        }
    }

    spdlog::critical("[FindConnectionHandleBySoldierId] Wrong soldier_id");
    return 0;
}

void PollGroupBase::EraseConnection(
  std::unordered_map<HSteamNetConnection, Connection>::iterator it_connection)
{
    connections_.erase(std::move(it_connection));
}

void PollGroupBase::SendStringToClient(HSteamNetConnection conn, const std::string& message)
{
    GetInterface()->SendMessageToConnection(
      conn, message.c_str(), message.size(), k_nSteamNetworkingSend_Reliable, nullptr);
}

void PollGroupBase::SendStringToAllClients(const std::string& message, HSteamNetConnection except)
{
    for (auto& c : connections_) {
        if (c.first != except) {
            SendStringToClient(c.first, message);
        }
    }
}

void PollGroupBase::SetClientNick(HSteamNetConnection h_conn, const std::string& nick)
{
    connections_[h_conn].nick = nick;
    // Set the connection name, too, which is useful for debugging
    GetInterface()->SetConnectionName(h_conn, nick.c_str());
}
} // namespace Soldank
