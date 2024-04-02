#pragma once
#include <SFML/Network.hpp>
#include <array>

#include "gameNetwork.h"

class GameServerLogic {
 public:
  GameState currentState = GameState::JoinLobby;
  std::string currentGuess = "";
  int connectedPlayers = 0;
  int turn = 0;
  int turnMax = 20;
  bool isSender = true;
  bool isWin = false;
  bool isPlayerOneTurn = true;
  bool isPlayerTurn = true;
  std::array<sf::TcpSocket*, 2> clients{nullptr, nullptr};
  std::string wordToFind = "";

  GameServerLogic() = default;

  void sendGameData(int socketSelector, bool playerRole);

  void receiveGuess(int socketSelector);

  void CheckCurrentGuess();

  void Update();

  void AddPlayer(sf::TcpSocket* player);
};