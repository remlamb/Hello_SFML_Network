#include "gameServerLogic.h"

void GameServerLogic::sendGameData(int socketSelector, bool playerRole) {
  sf::Packet packet;
  packet << isPlayerTurn << currentGuess << playerRole << isWin << turn;
  if (clients[socketSelector]->send(packet) == sf::Socket::Done) {
    std::cout << " Send for p1 : ";
    std::cout << isPlayerTurn << " Current guess: " << currentGuess
              << std::endl;
  } else {
    std::cerr << "Failed to send game data to client \n";
  }
}

void GameServerLogic::receiveGuess(int socketSelector) {
  sf::Packet packet;
  if (clients[socketSelector]->receive(packet) == sf::Socket::Done) {
    packet >> currentGuess >> isWin;
    std::cout << "Received guess : " << currentGuess << std::endl;
  }
}

void GameServerLogic::CheckCurrentGuess() {
  currentGuess = ToLower(currentGuess);
  if (currentGuess == wordToFind) {
    std::cout << "Correct!" << std::endl;
    isWin = true;
  } else {
    std::cout << "Incorrect!" << std::endl;
  }
}

void GameServerLogic::Run() {
  std::cout << "Player Log in";
  sf::Packet role;
  isSender = true;
  role << isSender;
  if (clients[0]->send(role) != sf::Socket::Done) {
    std::cerr << "Failed to send sender flag to client \n";
    // return EXIT_FAILURE;
  }

  isSender = false;
  role.clear();
  role << isSender;
  if (clients[1]->send(role) != sf::Socket::Done) {
    std::cerr << "Failed to send sender flag to client \n";
    // return EXIT_FAILURE;
  }
  currentState = GameState::SetSecretWord;

  if (currentState == GameState::SetSecretWord) {
    std::cout << "Waiting for word to find from client" << std::endl;

    sf::Packet secretWordPacket;
    if (clients[0]->receive(secretWordPacket) == sf::Socket::Done) {
      secretWordPacket >> wordToFind;
      currentState = GameState::FindingWord;
      std::cout << "Word to find: " << wordToFind << std::endl;
    } else {
      std::cerr << "Failed to receive word to find from client \n";
    }
  }

  if (currentState == GameState::FindingWord) {
    while (turn <= turnMax) {
      bool isPlayerSenderRole = false;

      sendGameData(0, isPlayerSenderRole);

      isPlayerTurn = !isPlayerTurn;
      isPlayerSenderRole = !isPlayerSenderRole;

      sendGameData(1, isPlayerSenderRole);

      if (isPlayerOneTurn) {
        receiveGuess(0);
      } else {
        receiveGuess(1);
        CheckCurrentGuess();
      }
      isPlayerOneTurn = !isPlayerOneTurn;
      isPlayerTurn = isPlayerOneTurn;
      turn++;
    }
  }
}
