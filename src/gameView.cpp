#include "gameView.h"

GameView::GameView() {
  texturePC = sf::Texture();
  font = sf::Font();
}

void GameView::Init() {
  if (!texturePC.loadFromFile("data/PCScreen.png")) {
    std::cerr << "Failed Loading Texture\n";
  }
  spritePC.setTexture(texturePC);

  if (!font.loadFromFile("data/Anta-Regular.ttf")) {
    std::cerr << "Failed Loading Font\n";
  }
  sf::Text message_send_draw("", font);
  text.setFont(font);
  text.setCharacterSize(20);
  message_send_draw.setPosition(firstMessagePosition);
  text.setPosition(firstMessagePosition);
  text.setString("<< " + game_logic_.userInput);

  window.create({800u, 600u}, "Turn Based Mini GameLogic", sf::Style::None);
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);
  game_logic_.Init();
}

void GameView::ManageEvent() {
  for (auto event = sf::Event{}; window.pollEvent(event);) {
    ImGui::SFML::ProcessEvent(event);
    if (event.type == sf::Event::Closed) {
      window.close();
    }
    game_logic_.ManageEvent(event);
    if (game_logic_.isPlayerTurn) {
      text.setPosition(GetNewPosition());
      text.setString("<< " + game_logic_.userInput);
    }
  }
}

void GameView::Update() {
  game_logic_.Update();
  while (window.isOpen()) {
    ManageEvent();
    // Render your SFML content here
    ImGui::SFML::Update(window, sf::seconds(1.0f / 60.0f));

    // ImGui widgets go here
    ImGui::Begin("Menu");
    if (ImGui::Button("Connect to Server")) {
      game_logic_.game_network.ConnectToServer();
      game_logic_.isSender = game_logic_.game_network.ReceiveRole();
      game_logic_.currentState = GameState::SetSecretWord;
    }
    ImGui::End();
    Render();
  }
  ImGui::SFML::Shutdown();
}

void GameView::Render() {
  window.clear();

  if (game_logic_.currentState == GameState::WinOrLoose) {
    if (game_logic_.isWordCorrect) {
      text.setString("Good Job");
      text.setColor(green);
    } else {
      text.setString("FAILED");
      text.setColor(orange);
    }

    text.setPosition(
        sf::Vector2f(window.getSize().x * 0.5, window.getSize().y * 0.5));
    text.setCharacterSize(40);
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width * 0.5f,
                   textRect.top + textRect.height * 0.5f);
    window.draw(text);
  }

  else if (game_logic_.currentState == GameState::SetSecretWord) {
    if (game_logic_.isSender) {
      sf::Text tuto_text("", font);
      tuto_text.setPosition(firstMessagePosition);
      tuto_text.setColor(green);
      tuto_text.setString(">> Enter the word you want P2 to find: ");
      text.setCharacterSize(20);
      sf::Vector2f offsetSecretWord = sf::Vector2f(20.0f, 40.0f);
      text.setPosition(firstMessagePosition + offsetSecretWord);
      window.draw(text);
      window.draw(tuto_text);
    } else {
      sf::Text tuto_text("", font);
      tuto_text.setPosition(firstMessagePosition);
      tuto_text.setColor(green);
      tuto_text.setString(">> Waiting For P1 Transmission...");
      text.setCharacterSize(20);
      window.draw(tuto_text);
    }

    text.setPosition(firstMessagePosition);
  }

  else if (game_logic_.currentState == GameState::FindingWord) {
    text.setCharacterSize(20);
    bool isFirstColorGreen = game_logic_.isGuesser ? false : true;
    for (int i = 1; i < messagesHistory.size(); i++) {
      sf::Text messageText("", font);
      if (isFirstColorGreen) {
        messageText.setString("  >> " + messagesHistory[i]);
        messageText.setColor(green);
      } else {
        messageText.setString("  << " + messagesHistory[i]);
        messageText.setColor(orange);
      }
      messageText.setPosition(GetPositionStr(i));
      window.draw(messageText);
      isFirstColorGreen = !isFirstColorGreen;
    }
    text.setPosition(GetNewPosition());
    window.draw(text);

    if (game_logic_.isPlayerTurn) {
      sf::Text Info("//Send something.. ", font);
      Info.setPosition(firstMessagePosition);
      Info.setColor(green);
      window.draw(Info);
    } else {
      sf::Text Info("//Waiting transmission.. ", font);
      Info.setPosition(firstMessagePosition);
      Info.setColor(green);
      window.draw(Info);
    }
  }

  window.draw(spritePC);

  if (game_logic_.currentState == GameState::JoinLobby) {
    ImGui::SFML::Render(window);
  }

  window.display();

  if (game_logic_.currentState == GameState::SetSecretWord) {
    if (!game_logic_.isSender) {
      game_logic_.currentState = GameState::FindingWord;
    }
  }

  if (game_logic_.currentState == GameState::FindingWord) {
    if (game_logic_.isWaitingForTurn) {
      game_logic_.game_network.WaitingForTurn(
          game_logic_.isPlayerTurn, game_logic_.wordReceived,
          game_logic_.isGuesser, game_logic_.isWordCorrect,
          game_logic_.currentTurn);
      sf::Text newMessage("<< " + game_logic_.userInput, font);
      sf::Vector2f newPosition = GetNewPosition();
      newMessage.setPosition(newPosition);
      newMessage.setColor(orange);
      newMessage.setString(game_logic_.wordReceived);
      messages.push_back(newMessage);
      messagesHistory.push_back(game_logic_.wordReceived);
      game_logic_.isWaitingForTurn = false;

      if (!game_logic_.isPlayerTurn) {
        game_logic_.isWaitingForTurn = true;
      }

      if (game_logic_.isWordCorrect ||
          game_logic_.currentTurn >= game_logic_.maxTurn) {
        game_logic_.currentState = GameState::WinOrLoose;
      }
    }
  }
}