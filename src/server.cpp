#include <SFML/Network.hpp>
#include <array>
#include <vector>
#include <iostream>

#include "gameNetwork.h"
#include "message.h"

class Server {
 public:
  GameState currentState = GameState::JoinLobby;
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
  std::string wordToFind;

  Server() {
    wordToFind = "";
    isSender = true;
  }

  void acceptClient(int socketSelector) {
    if (listener.accept(clients[socketSelector]) != sf::Socket::Done) {
      std::cerr << "Could not accept client \n";
      exit(EXIT_FAILURE);
    } else {
      std::cout << "Client connected: "
                << clients[socketSelector].getRemoteAddress() << ':'
                << clients[socketSelector].getRemotePort() << std::endl;
    }
  }

  void sendGameData(int socketSelector, bool playerRole) {
    sf::Packet packet;
    packet << isPlayerTurn << currentGuess << playerRole << isWin << turn;
    if (clients[socketSelector].send(packet) == sf::Socket::Done) {
      std::cout << " Send for p1 : ";
      std::cout << isPlayerTurn << " Current guess: " << currentGuess
                << std::endl;
    } else {
      std::cerr << "Failed to send game data to client \n";
    }
  }

  void receiveGuess(int socketSelector) {
    sf::Packet packet;
    if (clients[socketSelector].receive(packet) == sf::Socket::Done) {
      packet >> currentGuess >> isWin;
      std::cout << "Received guess : " << currentGuess << std::endl;
    }
  }

  void CheckCurrentGuess() {
    currentGuess = ToLower(currentGuess);
    if (currentGuess == wordToFind) {
      std::cout << "Correct!" << std::endl;
      isWin = true;
    } else {
      std::cout << "Incorrect!" << std::endl;
    }
  }

  void Run() {
    if (listener.listen(PORT) != sf::Socket::Done) {
      std::cerr << "Could not listen to port \n";
      // return EXIT_FAILURE;
    }
    std::cout << "Server running ! \n";

    while (true) {
      if (connectedPlayers < 2) {
        acceptClient(connectedPlayers);
        connectedPlayers++;
      }

      if (connectedPlayers > 1) {
        std::cout << "Player Log in";
        sf::Packet role;
        isSender = true;
        role << isSender;
        if (clients[0].send(role) != sf::Socket::Done) {
          std::cerr << "Failed to send sender flag to client \n";
          // return EXIT_FAILURE;
        }

        isSender = false;
        role.clear();
        role << isSender;
        if (clients[1].send(role) != sf::Socket::Done) {
          std::cerr << "Failed to send sender flag to client \n";
          // return EXIT_FAILURE;
        }
        currentState = GameState::SetSecretWord;
      }

      if (currentState == GameState::SetSecretWord) {
        std::cout << "Waiting for word to find from client" << std::endl;

        sf::Packet secretWordPacket;
        if (clients[0].receive(secretWordPacket) == sf::Socket::Done) {
          secretWordPacket >> wordToFind;
          currentState = GameState::FindingWord;
          std::cout << "Word to find: " << wordToFind << std::endl;
        } else {
          std::cerr << "Failed to receive word to find from client \n";
          // return EXIT_FAILURE;
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
  }
};

class Lobby
{
	
};

int main() {
  Server server;
  server.Run();
}