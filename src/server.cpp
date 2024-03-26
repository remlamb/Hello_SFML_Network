#include <SFML/Network.hpp>
#include <array>
#include <iostream>
#include <queue>
#include <vector>

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
  std::array<sf::TcpSocket*, 2> clients{};
  sf::TcpListener listener;
  std::string wordToFind;

  Server() {
    wordToFind = "";
    isSender = true;
  }

  Server(const Server& other)
      : currentState(other.currentState),
        currentGuess(other.currentGuess),
        connectedPlayers(other.connectedPlayers),
        turn(other.turn),
        turnMax(other.turnMax),
        isSender(other.isSender),
        isWin(other.isWin),
        isPlayerOneTurn(other.isPlayerOneTurn),
        isPlayerTurn(other.isPlayerTurn),
        clients{other.clients},
        wordToFind(other.wordToFind) {
    for (size_t i = 0; i < 2; ++i) {
      if (other.clients[i] != nullptr) {
        clients[i] = other.clients[i];
      } else {
        clients[i] = nullptr;
      }
    }
  }

  ~Server() {
    for (size_t i = 0; i < 2; ++i) {
      delete clients[i];
    }
  }

  void sendGameData(int socketSelector, bool playerRole) {
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

  void receiveGuess(int socketSelector) {
    sf::Packet packet;
    if (clients[socketSelector]->receive(packet) == sf::Socket::Done) {
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
    //while (true) {
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
  //}
};

class Lobby {
 public:
  std::queue<sf::TcpSocket*> clients{};
  std::vector<Server*> games{};
  sf::TcpListener listener;
  int GameCounter = 0;

  void Init() {
    clients.empty();
    games.clear();
  }

  void ListenToPort() {
    if (listener.listen(PORT) != sf::Socket::Done) {
      std::cerr << "Could not listen to port \n";
    }
    std::cout << "Server running ! \n";
  }

  void AcceptClient() {
    sf::TcpSocket* newClient = new sf::TcpSocket;
    if (listener.accept(*newClient) == sf::Socket::Done) {
      std::cout << "Client connected: " << newClient->getRemoteAddress() << ':'
                << newClient->getRemotePort() << std::endl;
      clients.push(newClient);
    } else {
      delete newClient;
      std::cerr << "Could not accept client" << std::endl;
    }
  }

  void AddGame() {
    if (clients.size() >= 2) {
      Server* serv = new Server();
      serv->clients[0] = clients.front();
      clients.pop();
      serv->clients[1] = clients.front();
      clients.pop();
      games.push_back(serv);
    }
  }

  void Run() {
    ListenToPort();

    while (true) {
      AcceptClient();
      AddGame();
      for (auto& game : games) {
        game->Run();
      }
    }
  }
};

int main() {
  // Server server;
  // server.Run();

  Lobby lobby;
  lobby.Init();
  lobby.Run();
  return 0;
}