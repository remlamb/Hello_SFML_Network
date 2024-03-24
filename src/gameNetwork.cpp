#include "gameNetwork.h"

void GameNetwork::ConnectToServer() {
  sf::Socket::Status status = socket.connect(HOST_NAME, PORT);
  if (status != sf::Socket::Done) {
    std::cerr << "Could not connect to server ! \n";
  }
}

std::string GameNetwork::ReceiveSecretWord() {
  sf::Packet packet;
  std::string receivedWord;
  if (socket.receive(packet) == sf::Socket::Done) {
    packet >> receivedWord;

    std::cout << "Word received from server" << std::endl;
  }
  return receivedWord;
}

bool GameNetwork::ReceiveRole() {
  sf::Packet packet;
  if (socket.receive(packet) != sf::Socket::Done) {
    std::cerr << "Failed to receive isSender flag from server" << std::endl;
  }
  bool sender;
  packet >> sender;
  std::cout << "Received isSender flag from server: " << std::boolalpha
            << sender << std::endl;
  return sender;
}

void GameNetwork::SendData(std::string userInput, bool isWordCorrect) {
  sf::Packet packetWithWord;
  userInput = ToLower(userInput);
  packetWithWord << userInput << isWordCorrect;
  if (socket.send(packetWithWord) == sf::Socket::Done) {
    std::cout << "Packet Send : " << std::endl;
  }
}

void GameNetwork::SendSecretWord(std::string secretWord) {
  std::cout << "enter Sending" << std::endl;
  sf::Packet packet;
  packet << secretWord;
  if (socket.send(packet) == sf::Socket::Done) {
    std::cout << "Packet Send" << std::endl;
  }
  std::cout << "End " << std::endl;
}

// TODO Attention outparameter
void GameNetwork::WaitingForTurn(bool& isPlayerTurn, std::string& wordReceived,
                                 bool& isGuesser, bool& isWordCorrect,
                                 int& currentTurn) {
  sf::Packet turnPacket;
  if (socket.receive(turnPacket) == sf::Socket::Done) {
    turnPacket >> isPlayerTurn >> wordReceived >> isGuesser >> isWordCorrect >>
        currentTurn;
    std::cout << "received  : " << isPlayerTurn << " Word" << std::endl;
  }
}