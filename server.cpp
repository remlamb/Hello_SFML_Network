#include <SFML/Network.hpp>
#include <iostream>
#include <array>
#include "message.h"

enum GameState {
	SetSecretWord,
	FindingWord,
	WinOrLoose,
};


void acceptClient(sf::TcpListener& listener, sf::TcpSocket& socket) {
	if (listener.accept(socket) != sf::Socket::Done) {
		std::cerr << "Could not accept client \n";
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Client connected: " << socket.getRemoteAddress() << ':' << socket.getRemotePort() << std::endl;
	}
}

void sendGameData(sf::TcpSocket& socket, bool isPlayerTurn, const std::string& currentGuess, bool playerRole, bool isWin, int turn) {
	sf::Packet packet;
	packet << isPlayerTurn << currentGuess << playerRole << isWin << turn;
	if (socket.send(packet) == sf::Socket::Done) {
		std::cout << " Send for p1 : ";
		std::cout << isPlayerTurn << " Current guess: " << currentGuess
			<< std::endl;
	}
	else
	{
		std::cerr << "Failed to send game data to client \n";
	}
}

void receiveGuess(sf::TcpSocket& socket, std::string& currentGuess, bool& isWin) {
	sf::Packet packet;
	if (socket.receive(packet) == sf::Socket::Done) {
		packet >> currentGuess >> isWin;
		std::cout << "Received guess : " << currentGuess << std::endl;
	}
}

int main() {
	GameState currentState = SetSecretWord;
	int connectedPlayers = 0;
	bool sender;
	bool isWin = false;
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

	std::array<sf::TcpSocket, 2> clients;
	while (true) {
		if (currentState == SetSecretWord) {
			if (connectedPlayers < 2) {
				acceptClient(listener, clients[connectedPlayers]);
				connectedPlayers++;
			}


			if (connectedPlayers > 1) {
				// Send the sender flag to the client
				sf::Packet packetRole;
				sender = true;
				packetRole << sender;
				if (clients[0].send(packetRole) != sf::Socket::Done) {
					std::cerr << "Failed to send sender flag to client \n";
					return EXIT_FAILURE;
				}


				sender = false;
				packetRole.clear();
				packetRole << sender;
				if (clients[1].send(packetRole) != sf::Socket::Done) {
					std::cerr << "Failed to send sender flag to client \n";
					return EXIT_FAILURE;
				}

				std::cout << "Waiting for word to find from client" << std::endl;
				std::string wordToFind;
				sf::Packet packet;
				if (clients[0].receive(packet) == sf::Socket::Done) {
					packet >> wordToFind;
					std::cout << "Word to find: " << wordToFind << std::endl;
				}
				else {
					std::cerr << "Failed to receive word to find from client \n";
					return EXIT_FAILURE;
				}

				sf::Packet packetWord;
				packetWord << wordToFind;
				if (clients[1].send(packetWord) == sf::Socket::Done) {
					std::cout << "Word send: " << wordToFind << std::endl;
					currentState = FindingWord;
				}
			}
		}

		if (currentState == FindingWord) {
			while (turn <= 20) {
				sf::Packet packet;
				sf::Packet packet2;
				sf::Packet packetReceived;
				bool playerRole = false;

				sendGameData(clients[0], isPlayerTurn, currentGuess, playerRole, isWin, turn);

				isPlayerTurn = !isPlayerTurn;
				playerRole = !playerRole;

				sendGameData(clients[1], isPlayerTurn, currentGuess, playerRole, isWin, turn);

				if (isPlayerOneTurn) {
					receiveGuess(clients[0], currentGuess, isWin);
				}
				else {
					receiveGuess(clients[1], currentGuess, isWin);
				}
				isPlayerOneTurn = !isPlayerOneTurn;
				isPlayerTurn = isPlayerOneTurn;
				turn++;
			}
		}
	}
}