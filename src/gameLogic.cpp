#include "gameLogic.h"

GameLogic::GameLogic() {
  isWaitingForTurn = true;
  isPlayerTurn = true;
  isWordCorrect = false;
  currentTurn = 0;
  isGuesser = false;
  isSender = false;
  wordReceived = "";
  currentState = GameState::JoinLobby;
  secretWord = "secret";
  userInput = "";
}

void GameLogic::Init() noexcept {
  //game_network.ConnectToServer();
  //isSender = game_network.ReceiveRole();
  isGuesser = false;
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

void GameLogic::ManageEvent(sf::Event event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Enter) {
      if (currentState == GameState::FindingWord) {
        // if (waiting..) continue
        if (!isWaitingForTurn) {
          if (isGuesser) {
            CheckCurrentTry();
            game_network.SendData(userInput, isWordCorrect);
            isWaitingForTurn = true;
            userInput = "";
            isGuesser = false;
          } else {
            userInput = ToLower(userInput);
            if (userInput == secretWord) {
              int worldSelected = RandomRange(0, 5);
              userInput = randomWords[worldSelected];
            }
            isGuesser = true;
            game_network.SendData(userInput, isWordCorrect);
            isWaitingForTurn = true;
            userInput = "";
          }
        }
      }
      if (currentState == GameState::SetSecretWord) {
        secretWord = userInput;
        secretWord = ToLower(secretWord);
        userInput = "";
        if (isSender) {
          game_network.SendSecretWord(secretWord);
          currentState = GameState::FindingWord;
        }
      }
    }

  } else if (event.type == sf::Event::TextEntered) {
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

void GameLogic::Update()
{

}
