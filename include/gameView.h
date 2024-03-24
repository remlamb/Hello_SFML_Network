#pragma once
#include "gameLogic.h"
#include <random>

class GameView {
  // TODO class color -> constexpr
  const sf::Color green = sf::Color(0.0f, 255.0f, 102.0f);
  const sf::Color orange = sf::Color(255, 175, 64);

  sf::Sprite spritePC;
  sf::Texture texturePC;
  sf::RenderWindow window;
  sf::Text text;
  sf::Font font;

  std::vector<sf::Text> messages;
  std::vector<std::string> messagesHistory;
  sf::Vector2f firstMessagePosition = sf::Vector2f(62.0f, 48.0f);
  float marginYText = 20.0f;

  GameLogic game_logic_{};

  sf::Vector2f GetNewPosition() const noexcept {
    return firstMessagePosition +
           sf::Vector2f(0.0f,
                        static_cast<float>(messages.size()) * marginYText);
  }

  sf::Vector2f GetPositionStr(const int i) const noexcept {
    return firstMessagePosition +
           sf::Vector2f(0.0f, static_cast<float>(i) * marginYText);
  }

 public:
  GameView();

  void Init();
  void ManageEvent();
  void Update();
  void Render();
};