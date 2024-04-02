#pragma once
#include <queue>
#include <vector>

#include "gameServerLogic.h"

class ServerNetworkManager {
 public:
  std::vector<std::unique_ptr<sf::TcpSocket>> clients{};
  std::vector<GameServerLogic> games{};
  sf::TcpListener listener;
  sf::SocketSelector selector;

  void Init();
  void ListenToPort();
  void AcceptClient();
  void AddGame();
  void Run();
};