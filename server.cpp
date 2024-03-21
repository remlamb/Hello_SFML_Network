#include <SFML/Network.hpp>
#include <array>
#include <iostream>

#include "message.h"

enum class GameState {
  SetSecretWord,
  FindingWord,
  WinOrLoose,
};

void acceptClient(sf::TcpListener& listener, sf::TcpSocket& socket) {
  if (listener.accept(socket) != sf::Socket::Done) {
    std::cerr << "Could not accept client \n";
    exit(EXIT_FAILURE);
  } else {
    std::cout << "Client connected: " << socket.getRemoteAddress() << ':'
              << socket.getRemotePort() << std::endl;
  }
}

void sendGameData(sf::TcpSocket& socket, bool isPlayerTurn,
                  const std::string& currentGuess, bool playerRole, bool isWin,
                  int turn) {
  sf::Packet packet;
  packet << isPlayerTurn << currentGuess << playerRole << isWin << turn;
  if (socket.send(packet) == sf::Socket::Done) {
    std::cout << " Send for p1 : ";
    std::cout << isPlayerTurn << " Current guess: " << currentGuess
              << std::endl;
  } else {
    std::cerr << "Failed to send game data to client \n";
  }
}

void receiveGuess(sf::TcpSocket& socket, std::string& currentGuess,
                  bool& isWin) {
  sf::Packet packet;
  if (socket.receive(packet) == sf::Socket::Done) {
    packet >> currentGuess >> isWin;
    std::cout << "Received guess : " << currentGuess << std::endl;
  }
}

int main() {
  GameState currentState = GameState::SetSecretWord;
  std::string currentGuess = "";
  int connectedPlayers = 0;
  int turn = 0;
  int turnMax = 20;
  bool isSender;
  bool isWin = false;
  bool isPlayerOneTurn = true;
  bool isPlayerTurn = true;
  std::array<sf::TcpSocket, 2> clients;
  sf::TcpListener listener;

  if (listener.listen(PORT) != sf::Socket::Done) {
    std::cerr << "Could not listen to port \n";
    return EXIT_FAILURE;
  }
  std::cout << "Server running ! \n";

  while (true) {
    if (currentState == GameState::SetSecretWord) {
      if (connectedPlayers < 2) {
        acceptClient(listener, clients[connectedPlayers]);
        connectedPlayers++;
      }

      if (connectedPlayers > 1) {
        sf::Packet role;
        isSender = true;
        role << isSender;
        if (clients[0].send(role) != sf::Socket::Done) {
          std::cerr << "Failed to send sender flag to client \n";
          return EXIT_FAILURE;
        }

        isSender = false;
        role.clear();
        role << isSender;
        if (clients[1].send(role) != sf::Socket::Done) {
          std::cerr << "Failed to send sender flag to client \n";
          return EXIT_FAILURE;
        }

        std::cout << "Waiting for word to find from client" << std::endl;
        std::string wordToFind;
        sf::Packet secretWordPacket;
        if (clients[0].receive(secretWordPacket) == sf::Socket::Done) {
          secretWordPacket >> wordToFind;
          std::cout << "Word to find: " << wordToFind << std::endl;
        } else {
          std::cerr << "Failed to receive word to find from client \n";
          return EXIT_FAILURE;
        }

        secretWordPacket.clear();
        secretWordPacket << wordToFind;
        if (clients[1].send(secretWordPacket) == sf::Socket::Done) {
          std::cout << "Word send: " << wordToFind << std::endl;
          currentState = GameState::FindingWord;
        }
      }
    }

    if (currentState == GameState::FindingWord) {
      while (turn <= turnMax) {
        bool isPlayerSenderRole = false;

        sendGameData(clients[0], isPlayerTurn, currentGuess, isPlayerSenderRole,
                     isWin, turn);

        isPlayerTurn = !isPlayerTurn;
        isPlayerSenderRole = !isPlayerSenderRole;

        sendGameData(clients[1], isPlayerTurn, currentGuess, isPlayerSenderRole,
                     isWin, turn);

        if (isPlayerOneTurn) {
          receiveGuess(clients[0], currentGuess, isWin);
        } else {
          receiveGuess(clients[1], currentGuess, isWin);
        }
        isPlayerOneTurn = !isPlayerOneTurn;
        isPlayerTurn = isPlayerOneTurn;
        turn++;
      }
    }
  }
}