#include <SFML/Network.hpp>
#include <iostream>

#include "message.h"

enum GameState {
  SetSecretWord,
  FindingWord,
  WinOrLoose,
};

int main() {
  GameState currentState = SetSecretWord;
  int connectedPlayers = 0;
  bool sender;
  bool isWin = false;
  // 2nd Part
  bool isPlayerOneTurn = true;
  bool isPlayerTurn = true;
  std::string currentGuess = "";
  int turn = 0;

  sf::TcpListener listener;
  // Bind listener to a port
  if (listener.listen(12000) != sf::Socket::Done) {
    // error....
    std::cerr << "Could not listen to port \n";
    return EXIT_FAILURE;
  }
  std::cout << "Server running ! \n";
  while (true) {
    sf::TcpSocket client1;
    sf::TcpSocket client2;
    if (currentState == SetSecretWord) {
      if (connectedPlayers == 0) {
        // accept a new connection

        if (listener.accept(client1) != sf::Socket::Done) {
          // error
          std::cerr << "Could not accept client \n";
          return EXIT_FAILURE;
        } else {
          connectedPlayers++;
          std::cout << "Client1 :" << client1.getRemoteAddress() << ':'
                    << client1.getRemotePort() << "is connected \n";
        }
      }

      if (connectedPlayers == 1) {
        if (listener.accept(client2) != sf::Socket::Done) {
          // error
          std::cerr << "Could not accept client \n";
          return EXIT_FAILURE;
        } else {
          connectedPlayers++;
          std::cout << "Client2" << client2.getRemoteAddress() << ':'
                    << client2.getRemotePort() << "is connected \n";
        }
      }

      // NEW
      if (connectedPlayers > 1) {
        // Send the sender flag to the client
        sf::Packet packetRole;
        sender = true;
        packetRole << sender;
        if (client1.send(packetRole) != sf::Socket::Done) {
          std::cerr << "Failed to send sender flag to client \n";
          return EXIT_FAILURE;
        }
        sender = false;
        packetRole.clear();
        packetRole << sender;
        if (client2.send(packetRole) != sf::Socket::Done) {
          std::cerr << "Failed to send sender flag to client \n";
          return EXIT_FAILURE;
        }

        std::cout << "Waiting for word to find from client" << std::endl;
        std::string wordToFind;
        sf::Packet packet;
        if (client1.receive(packet) == sf::Socket::Done) {
          packet >> wordToFind;
          std::cout << "Word to find: " << wordToFind << std::endl;
        } else {
          std::cerr << "Failed to receive word to find from client \n";
          return EXIT_FAILURE;
        }

        sf::Packet packetWord;
        packetWord << wordToFind;
        if (client2.send(packetWord) == sf::Socket::Done) {
          std::cout << "Word send: " << wordToFind << std::endl;
          currentState = FindingWord;
        }
      }
    }

    if (currentState == FindingWord) {
      while (turn <= 10) {
        sf::Packet packet;
        sf::Packet packet2;
        sf::Packet packetReceived;
        bool playerRole = false;
        packet.clear();
        packet << isPlayerTurn << currentGuess << playerRole << isWin;
        if (client1.send(packet) == sf::Socket::Done) {
          std::cout << " Send for p1 : ";
          std::cout << isPlayerTurn << " Current guess: " << currentGuess
                    << std::endl;
        }
        packet.clear();
        isPlayerTurn = !isPlayerTurn;
        playerRole = !playerRole;
        packet2 << isPlayerTurn << currentGuess << playerRole << isWin;
        if (client2.send(packet2) == sf::Socket::Done) {
          std::cout << " Send for p2 : ";
          std::cout << isPlayerTurn << " Current guess: " << currentGuess
                    << std::endl;
        }

        packetReceived.clear();
        if (isPlayerOneTurn) {
          if (client1.receive(packetReceived) == sf::Socket::Done) {
            packetReceived >> currentGuess >> isWin;
            std::cout << "Received string: " << currentGuess << std::endl;
          }
        } else {
          if (client2.receive(packetReceived) == sf::Socket::Done) {
            packetReceived >> currentGuess >> isWin;
            std::cout << "Received string: " << currentGuess << std::endl;
          }
        }
        isPlayerOneTurn = !isPlayerOneTurn;
        isPlayerTurn = isPlayerOneTurn;
      }
    }
  }
}