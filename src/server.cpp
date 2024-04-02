#include <list>

#include "ServerNetworkManager.h"
#include "message.h"

sf::Packet& operator>>(sf::Packet& packet, GameState& state) {
  int value;
  packet >> value;
  state = static_cast<GameState>(value);
  return packet;
}

int main() {
  bool connectionStatus = false;
  sf::TcpListener listener;

  std::string message;
  std::string secretWord;
  bool isSender = false;
  bool isWin = false;
  GameState packetGameState = GameState::JoinLobby;
  listener.listen(PORT);

  // Create a list to store the future clients
  std::list<sf::TcpSocket*> clients;

  // Create a selector
  sf::SocketSelector selector;

  // Add the listener to the selector
  selector.add(listener);

  // Endless loop that waits for new connections
  while (true) {
    // Make the selector wait for data on any socket
    if (selector.wait(sf::seconds(5))) {
      // Test the listener
      if (selector.isReady(listener)) {
        std::cout << "Listener is ready: accepting new connection..."
                  << std::endl;
        sf::TcpSocket* client = new sf::TcpSocket;
        if (listener.accept(*client) == sf::Socket::Done) {
          std::cout << "New connection accepted." << std::endl;
          client->setBlocking(false);

          sf::Packet connectionPacket;
          connectionStatus = !connectionStatus;  // Adjust as needed
          std::cout << connectionStatus;
          connectionPacket << connectionStatus;
          // if (client->send(connectionPacket) != sf::Socket::Done) {
          //   std::cerr << "Failed to send connection status to client!"
          //             << std::endl;
          // }
          sf::Socket::Status status = sf::Socket::Partial;
          do {
            status = client->send(connectionPacket);
          } while (status == sf::Socket::Partial);

          clients.push_back(client);

          // Add the new client to the selector so that we will
          // be notified when he sends something
          selector.add(*client);
          std::cout << "client Added to Selector";

        } else {
          delete client;
          std::cerr << "Failed to accept new connection." << std::endl;
        }
      } else {
        // The listener socket is not ready, test all other sockets (the
        // clients)
        for (std::list<sf::TcpSocket*>::iterator it = clients.begin();
             it != clients.end(); ++it) {
          sf::TcpSocket& client = **it;
          if (selector.isReady(client)) {
            // send Packet
            sf::Packet packet;
            sf::Socket::Status status = sf::Socket::Partial;
            do {
              status = client.receive(packet);
              if (status == sf::Socket::Status::NotReady) {
                break;
              }
              if (status == sf::Socket::Done) {
                packet >> packetGameState;
                switch (packetGameState) {
                  case GameState::JoinLobby:
                    std::cout << "Lobby" << std::endl;
                    break;
                  case GameState::SetSecretWord:
                    std::cout << " SetSecretWord " << std::endl;
                    break;
                  case GameState::FindingWord:
                    std::cout << "FindingWord " << std::endl;
                    break;
                  case GameState::WinOrLoose:
                    isWin = true;
                    break;
                  case sf::Socket::Error:
                    break;
                }
                packet >> message;
                packet >> isSender;
                std::cout << "Received message from client: " << message
                          << std::endl;
              }
            } while (status == sf::Socket::Partial);

            if (packetGameState == GameState::SetSecretWord) {
              secretWord = message;
              std::cout << "secretWord is: " << secretWord;
            }

            if (packetGameState == GameState::FindingWord) {
              std::cout << secretWord << " msg: " << message << std::endl;
              if (!isSender) {
                std::cout << "isGuesserTurn" << std::endl;
                if (message == secretWord) {
                  isWin = true;
                  std::cout << "isWin: " << isWin << std::endl;
                }
              }
            }

            packet.clear();
            packet << message;
            packet << isWin;
            for (auto otherClientIt = clients.begin();
                 otherClientIt != clients.end(); ++otherClientIt) {
              if (*otherClientIt !=
                  *it) {  // Ne pas renvoyer au client d'origine
                sf::TcpSocket& otherClient = **otherClientIt;
                status = sf::Socket::Partial;
                do {
                  status = otherClient.send(packet);
                  if (status == sf::Socket::Done) {
                    std::cout << "Message send to client: " << message
                              << std::endl;
                  }
                  if (status == sf::Socket::Status::NotReady) {
                    break;
                  }
                } while (status == sf::Socket::Partial);
              }
            }
          }
        }
      }
    }
  }
  return 0;
}