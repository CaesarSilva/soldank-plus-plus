#include "networking/interface/NetworkingInterface.hpp"

#include <vector>
#include <functional>
#include <iostream>
#include <array>

namespace Soldat::NetworkingInterface
{
namespace
{
ISteamNetworkingSockets* interface;
std::vector<std::function<void(SteamNetConnectionStatusChangedCallback_t*)>> observers;

void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* p_info)
{
    for (const auto& observer : observers) {
        observer(p_info);
    }
}
} // namespace

void Init()
{
    interface = SteamNetworkingSockets();
}

std::shared_ptr<Connection> CreateConnection()
{
    SteamNetworkingIPAddr server_addr{};
    server_addr.Clear();
    server_addr.ParseString("127.0.0.1");
    server_addr.m_port = 23073;
    std::cout << "Connecting to server" << std::endl;

    SteamNetworkingConfigValue_t opt{};
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void*)SteamNetConnectionStatusChangedCallback);
    HSteamNetConnection connection_handle = interface->ConnectByIPAddress(server_addr, 1, &opt);
    if (connection_handle == k_HSteamNetConnection_Invalid) {
        std::cout << "Failed to create connection" << std::endl;
    }

    return std::make_shared<Connection>(interface, connection_handle);
}

void PollConnectionStateChanges()
{
    interface->RunCallbacks();
}

void RegisterObserver(
  const std::function<void(SteamNetConnectionStatusChangedCallback_t*)>& observer)
{
    observers.push_back(observer);
}

void Free()
{
    observers.clear();
}
} // namespace Soldat::NetworkingInterface