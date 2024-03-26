#include "lobby.h"

void Lobby::Init() {
  clients.empty();
  games.clear();
}

void Lobby::ListenToPort() {
  if (listener.listen(PORT) != sf::Socket::Done) {
    std::cerr << "Could not listen to port \n";
  }
  std::cout << "GameServerLogic running ! \n";
}

void Lobby::AcceptClient() {
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

void Lobby::AddGame() {
  if (clients.size() >= 2) {
    GameServerLogic* serv = new GameServerLogic();
    serv->clients[0] = clients.front();
    clients.pop();
    serv->clients[1] = clients.front();
    clients.pop();
    games.push_back(serv);
  }
}

void Lobby::Run() {
  ListenToPort();

  while (true) {
    AcceptClient();
    AddGame();
    for (auto& game : games) {
      game->Run();
    }
  }
}