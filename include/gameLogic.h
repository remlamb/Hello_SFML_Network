#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <array>
#include <iostream>

#include "gameNetwork.h"

class GameLogic {
 public:
  int currentTurn = 0;
  static constexpr int maxTurn = 20;
  bool isGuesserTurn = false;
  bool isSender = true;
  bool isWaitingForTurn = true;
  bool isPlayerTurn = true;
  bool isWordCorrect = false;
  bool isSenderTheGuesser = true;
  std::string wordReceived;
  std::string secretWord = "secret";
  std::string userInput;
  GameNetwork game_network{};
  std::vector<std::string> messagesHistory;
  GameState currentState = GameState::SetSecretWord;
  static constexpr std::array<std::string_view, 6> randomWords = {
      "Oui", "Chien", "SFML_Classic", "reel_", "rouge", "random"};

  GameLogic();
  void Init() noexcept;
  void CheckCurrentTry();
  void ManageEvent(sf::Event event);

  void OnEvent(sf::Event event);
  void Update();
};