#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>

#include "common.h"

enum class GameState {
  JoinLobby = 0,
  SetSecretWord,
  FindingWord,
  WinOrLoose,
};

struct GameData {
  bool isPlayerTurn;
  std::string lastWord;
  bool isWin;
};

class GameNetwork {
 public:
  sf::TcpSocket socket;
  bool roleReceived = false;

  GameNetwork() {}

  void ConnectToServer();
  bool ReceiveRole();
  void SendGameData(bool& isPlayerTurn, std::string& wordReceived,
                    bool& isGuesser, bool& isWordCorrect, int& currentTurn);
  void ReceiveGameData(bool& isPlayerTurn, std::string& wordReceived,
                       bool& isGuesser, bool& isWordCorrect, int& currentTurn);

  void SendData(std::string secretWord, bool& isTurn,
                      std::vector<std::string>& vector, GameState& state,
                      bool& isGuesserTheSender);

  std::string ReceiveData(bool& IsTurn, std::vector<std::string>& vector,
                                bool& isWin);
};
