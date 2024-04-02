#include "gameLogic.h"

GameLogic::GameLogic() {
  isWaitingForTurn = true;
  isPlayerTurn = true;
  isWordCorrect = false;
  currentTurn = 0;
  isGuesserTurn = false;
  isSender = false;
  wordReceived = "";
  currentState = GameState::JoinLobby;
  secretWord = "secret";
  userInput = "";
}

void GameLogic::Init() noexcept {
  isGuesserTurn = false;
}

void GameLogic::CheckCurrentTry() {
  std::cout << "Message Entered" << std::endl;
  userInput = ToLower(userInput);
  if (userInput == secretWord) {
    std::cout << "Correct!" << std::endl;
    isWordCorrect = true;
  } else {
    std::cout << "Incorrect!" << std::endl;
  }
}

void GameLogic::OnEvent(sf::Event event) {
  if (event.type == sf::Event::KeyPressed) {
    if (isWaitingForTurn) {
      return;
    }
    if (event.key.code == sf::Keyboard::Enter) {
      if (currentState == GameState::FindingWord) {
        if (isGuesserTurn) {
          userInput = ToLower(userInput);
          game_network.SendData(userInput, isWaitingForTurn,
                                      messagesHistory, currentState, isSender);
          isWaitingForTurn = true;
          userInput = "";
          // isGuesserTurn = false;
        } else {
          userInput = ToLower(userInput);
          if (userInput == secretWord) {
            int worldSelected = RandomRange(0, 5);
            userInput = randomWords[worldSelected];
          }
          game_network.SendData(userInput, isWaitingForTurn,
                                      messagesHistory, currentState, isSender);
          // isGuesserTurn = true;
          isWaitingForTurn = true;
          userInput = "";
        }
      }
      if (currentState == GameState::SetSecretWord) {
        secretWord = ToLower(userInput);
        userInput = "";
        game_network.SendData(secretWord, isWaitingForTurn,
                                    messagesHistory, currentState, isSender);

        currentState = GameState::FindingWord;
      }
    }
  }
  if (event.type == sf::Event::TextEntered) {
    if (isWaitingForTurn) {
      return;
    }
    if (!isWaitingForTurn || currentState == GameState::SetSecretWord) {
      if (isPlayerTurn) {
        if (event.text.unicode < 128 && event.text.unicode > 32) {
          userInput += static_cast<char>(event.text.unicode);
        } else if (event.text.unicode == 32) {
          userInput += '_';
        } else if (event.text.unicode == 8) {
          if (!userInput.empty()) {
            userInput.pop_back();
          }
        }
      }
    }
  }
}

// TODO
void GameLogic::Update() {
  // std::cout << static_cast<int>(currentState) << std::endl;
  if (currentState == GameState::SetSecretWord) {
    if (!isSender) {
      currentState = GameState::FindingWord;
    }
  }

  if (currentState == GameState::FindingWord) {
    if (isWaitingForTurn) {
      game_network.ReceiveData(isWaitingForTurn, messagesHistory,
                                     isWordCorrect);

      if (isWordCorrect || currentTurn >= maxTurn) {
        currentState = GameState::WinOrLoose;
      }
    }
  }

  if (currentState == GameState::WinOrLoose) {
    if (isSender) {
      game_network.SendData(secretWord, isWaitingForTurn, messagesHistory,
                            currentState, isSender);
    }
    currentState = GameState::WinOrLoose;
  }
}
