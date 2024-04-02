#include "ServerNetworkManager.h"

void ServerNetworkManager::Init() {
  clients.clear();
  games.clear();
}

void ServerNetworkManager::ListenToPort() {
  if (listener.listen(PORT) != sf::Socket::Done) {
    std::cerr << "Could not listen to port \n";
  }
  listener.setBlocking(false);
  selector.add(listener);
  std::cout << "server is  running ! \n";
}

void ServerNetworkManager::AcceptClient() {
  clients.emplace_back(std::make_unique<sf::TcpSocket>());
  const auto& client = clients.back();
  client->setBlocking(false);
  if (listener.accept(*client) == sf::Socket::Done) {
    selector.add(*client);
  }
}

// void ServerNetworkManager::AddGame() {
//   if (clients.size() >= 2) {
//     GameServerLogic* serv = new GameServerLogic();
//     serv->clients[0] = clients.front();
//     clients.pop();
//     serv->clients[1] = clients.front();
//     clients.pop();
//     games.push_back(serv);
//   }
// }

void ServerNetworkManager::Run() {
  ListenToPort();

  while (true) {
    // Make the selector wait for data on any socket
    if (selector.wait(sf::seconds(3))) {
      // Test the listener
      if (selector.isReady(listener)) {
        // The listener is ready: there is a pending connection
        clients.emplace_back(std::make_unique<sf::TcpSocket>());
        const auto& client = clients.back();
        client->setBlocking(false);
        if (listener.accept(*client) == sf::Socket::Done) {
          selector.add(*client);
        }
        const auto it = std::find_if(games.begin(), games.end(),
                                     [](const GameServerLogic& game) {
                                       return !(game.clients[0] != nullptr &&
                                                game.clients[1] != nullptr);
                                     });
        if (it != games.end()) {
          auto& game = *it;
          game.AddPlayer(client.get());
        } else {
          games.emplace_back(GameServerLogic());
          games.back().AddPlayer(client.get());
        }
      }
    } else {
      sf::Packet client_packet;
      for (auto& client : clients) {
        if (selector.isReady(*client)) {
          // Logic
          sf::Socket::Status status = sf::Socket::Partial;
          do {
            status = client->receive(client_packet);
          } while (status == sf::Socket::Partial);
        }
      }
    }
  }
}
