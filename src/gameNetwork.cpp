#include "gameNetwork.h"

sf::Packet& operator<<(sf::Packet& packet, const GameState& state) {
  return packet << static_cast<int>(state);
}

void GameNetwork::ConnectToServer() {
  sf::Socket::Status status = socket.connect(HOST_NAME, PORT);
  if (status != sf::Socket::Done) {
    std::cerr << "Could not connect to server ! \n";
  }
  socket.setBlocking(false);
}

bool GameNetwork::ReceiveRole() {
  sf::Packet packet;
  bool sender = false;
  sf::Socket::Status status = sf::Socket::Partial;
  while (!roleReceived) {
    status = socket.receive(packet);
    if (status == sf::Socket::Done) {
      packet >> sender;
      std::cout << "Received"
                << " " << packet << " " << sender << std::endl;
      roleReceived = true;
    } else {
      std::cout << "Not received" << std::endl;
    }
  }
  return sender;
}

void GameNetwork::SendData(std::string secretWord, bool& isTurn,
                           std::vector<std::string>& vector, GameState& state,
                           bool& isGuesserTheSender) {
  sf::Packet packet;
  packet << state;
  packet << secretWord;
  packet << isGuesserTheSender;
  std::cout << "isSender: " << isGuesserTheSender << std::endl;
  sf::Socket::Status status = sf::Socket::Partial;
  do {
    status = socket.send(packet);
    if (status == sf::Socket::Done) {
      state = GameState::FindingWord;
      vector.emplace_back(secretWord);
      //std::cout << "Packet Send" << secretWord << std::endl;
      isTurn = true;
    } else if (status == sf::Socket::Partial) {
      //std::cout << "Partial packet Send" << std::endl;
    } else {
      //std::cout << "Packet error Send" << std::endl;
    }
  } while (status == sf::Socket::Partial);
}

void GameNetwork::ReceiveGameData(bool& isPlayerTurn, std::string& wordReceived,
                                  bool& isGuesser, bool& isWordCorrect,
                                  int& currentTurn) {
  sf::Packet turnPacket;
  if (socket.receive(turnPacket) == sf::Socket::Done) {
    turnPacket >> isPlayerTurn >> wordReceived >> isGuesser >> isWordCorrect >>
        currentTurn;
    std::cout << "received  : " << isPlayerTurn << " Word" << std::endl;
  }
}

std::string GameNetwork::ReceiveData(bool& isTurn,
                                     std::vector<std::string>& vector,
                                     bool& isWin) {
  sf::Packet packet;
  std::string receivedWord;
  std::cout << "Want received" << receivedWord << std::endl;
  sf::Socket::Status status = sf::Socket::Partial;
  do {
    status = socket.receive(packet);
    if (status == sf::Socket::Done) {
      packet >> receivedWord;
      vector.emplace_back(receivedWord);
      std::cout << "Word received from server" << receivedWord << std::endl;
      isTurn = false;
      packet >> isWin;
      std::cout << isWin << std::endl;
    } else if (status == sf::Socket::Partial) {
      std::cout << "Partial packet Received" << std::endl;
    } else if (status == sf::Socket::NotReady) {
      break;
    }
  } while (status == sf::Socket::Partial);
  return receivedWord;
}

void GameNetwork::SendGameData(bool& isPlayerTurn, std::string& wordReceived,
                               bool& isGuesser, bool& isWordCorrect,
                               int& currentTurn) {
  sf::Packet packetWithWord;
  wordReceived = ToLower(wordReceived);
  packetWithWord << isPlayerTurn << wordReceived << isGuesser << isWordCorrect
                 << currentTurn;
  if (socket.send(packetWithWord) == sf::Socket::Done) {
    std::cout << "Packet Send : " << std::endl;
  }
}