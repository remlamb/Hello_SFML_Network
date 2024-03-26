#pragma once
#include "gameServerLogic.h"
#include <queue>
#include <vector>

class Lobby {
 public:
  std::queue<sf::TcpSocket*> clients{};
  std::vector<GameServerLogic*> games{};
  sf::TcpListener listener;
  int GameCounter = 0;

  void Init();
  void ListenToPort();
  void AcceptClient();
  void AddGame();
  void Run();
};