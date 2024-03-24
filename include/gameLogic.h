#pragma once
#include "gameNetwork.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <array>

class GameLogic {
 public:
  int currentTurn = 0;
  static constexpr int maxTurn = 20;
  bool isGuesser = false;
  bool isSender = false;
  bool isWaitingForTurn = true;
  bool isPlayerTurn = true;
  bool isWordCorrect = false;
  std::string wordReceived;
  std::string secretWord = "secret";
  std::string userInput;
  GameNetwork game_network{};
  GameState currentState = GameState::SetSecretWord;
  static constexpr std::array<std::string_view, 6> randomWords = {
      "Oui", "Chien", "SFML_Classic", "reel_", "rouge", "random"};

  GameLogic();
  void Init() noexcept;
  void Update(){};
  void CheckCurrentTry();
  void ManageEvent(sf::Event event);
};