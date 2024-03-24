#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include "common.h"

enum class GameState {
  SetSecretWord,
  FindingWord,
  WinOrLoose,
};

class GameNetwork {
 public:
  sf::TcpSocket socket;

  GameNetwork() {}

  void ConnectToServer();

  std::string ReceiveSecretWord();

  bool ReceiveRole();

  void SendData(std::string userInput, bool isWordCorrect);

  // Doit etre coté serveur ne pas partager le secret word
  void SendSecretWord(std::string secretWord);
  // TODO Attention outparameter
  void WaitingForTurn(bool& isPlayerTurn, std::string& wordReceived,
                      bool& isGuesser, bool& isWordCorrect, int& currentTurn);
};
