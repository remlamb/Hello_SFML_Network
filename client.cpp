#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>

#include "message.h"

struct Font {
  sf::Font font;
  int size;
  void LoadFont() {
    if (!font.loadFromFile("../Anta-Regular.ttf")) {
      std::cerr << "Failed to load font" << std::endl;
    }
  }
  Font(int size) {
    LoadFont();
    this->size = size;
  }
};

sf::Color green(0.0f, 255.0f, 102.0f);
sf::Color orange(255, 175, 64);
std::vector<sf::Text> messages;
std::vector<std::string> messagess;
sf::Vector2f firstMessagePosition(10.f, 10.f);
float marginYText = 20.0f;
sf::Vector2f GetNewPosition() {
  return firstMessagePosition +
         sf::Vector2f(0.0f, static_cast<float>(messages.size()) * marginYText);
}

sf::Vector2f GetPositionStr(int i) {
  return firstMessagePosition +
         sf::Vector2f(0.0f, static_cast<float>(i) * marginYText);
}

bool CompareString(std::string str1, std::string str2) {
  if (str1.size() != str2.size()) {
    return false;
  }
  for (size_t i = 0; i < str1.size(); ++i) {
    if (str1[i] != str2[i]) {
      return false;
    }
  }
  return true;
}

enum GameState {
  SetSecretWord,
  FindingWord,
  WinOrLoose,
};

int main() {
  bool isWaitingForTurn = true;
  bool isPlayerTurn = true;
  bool isWordCorrect = false;
  int currentTurn = 0;
  std::string wordReceived;
  sf::TcpSocket socket;
  sf::Socket::Status status = socket.connect(HOST_NAME, PORT);
  if (status != sf::Socket::Done) {
    // error
    std::cerr << "Could not connect to server ! \n";
    return EXIT_FAILURE;
  }

  bool sender;
  sf::Packet packet;
  if (socket.receive(packet) != sf::Socket::Done) {
    std::cerr << "Failed to receive sender flag from server" << std::endl;
    return EXIT_FAILURE;
  }
  packet >> sender;
  std::cout << "Received sender flag from server: " << std::boolalpha << sender
            << std::endl;

  Font font(20);
  GameState current_state = SetSecretWord;
  sf::Text message_send_draw("", font.font, font.size);
  bool isGuesser = false;

  sf::Text text("", font.font, font.size);
  message_send_draw.setPosition(firstMessagePosition);
  text.setPosition(firstMessagePosition);

  std::string secretWord = "secret";

  std::string inputText;
  text.setString("<< " + inputText);

  auto window =
      sf::RenderWindow{{800u, 600u}, "Turn Based Mini Game", sf::Style::Close};
  window.setFramerateLimit(60);

  while (window.isOpen()) {
    for (auto event = sf::Event{}; window.pollEvent(event);) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
          if (current_state == GameState::FindingWord) {
            if (!isWaitingForTurn) {
              if (isGuesser) {
                std::cout << "Message Entered" << std::endl;
                if (inputText == secretWord) {
                  std::cout << "Correct!" << std::endl;
                  isWordCorrect = true;
                  // current_state = GameState::WinOrLoose;
                } else {
                  std::cout << "Incorrect!" << std::endl;
                  std::cout << inputText + " - " + secretWord + "-"
                            << std::endl;
                }
                sf::Text newMessage(">> " + inputText, font.font, font.size);
                sf::Vector2f newPosition = GetNewPosition();
                newMessage.setPosition(newPosition);
                newMessage.setColor(green);
                text.setPosition(GetNewPosition());

                sf::Packet packetWithWord;
                packetWithWord << inputText << isWordCorrect;
                if (socket.send(packetWithWord) == sf::Socket::Done) {
                  std::cout << "Packet Send : " << inputText << std::endl;
                  isWaitingForTurn = true;
                }
                inputText = "";
                text.setString(">> " + inputText);
                isGuesser = false;
              } else {
                if (inputText == secretWord) {
                  inputText = "random";  // TODO
                }
                sf::Text newMessage("<< " + inputText, font.font, font.size);
                sf::Vector2f newPosition = GetNewPosition();
                newMessage.setPosition(newPosition);
                newMessage.setColor(orange);
                text.setPosition(GetNewPosition());

                isGuesser = true;

                sf::Packet packetWithWord;
                packetWithWord << inputText << isWordCorrect;
                if (socket.send(packetWithWord) == sf::Socket::Done) {
                  std::cout << "Packet Send : " << inputText << std::endl;
                  isWaitingForTurn = true;
                }
                inputText = "";
                text.setString(">> " + inputText);
              }
            }
          }
          if (current_state == GameState::SetSecretWord) {
            secretWord = inputText;
            inputText = "";
            text.setString("<< " + inputText);

            if (sender) {
              std::cout << "enter Sending" << std::endl;
              sf::Packet packet;
              packet << secretWord;
              if (socket.send(packet) == sf::Socket::Done) {
                std::cout << "Packet Send" << std::endl;
                current_state = GameState::FindingWord;
              }
              std::cout << "End " << std::endl;
            }
          }
        }

      } else if (event.type == sf::Event::TextEntered) {
        if (!isWaitingForTurn || current_state == SetSecretWord) {
          if (isPlayerTurn) {
            text.setPosition(GetNewPosition());
            if (event.text.unicode < 128 && event.text.unicode > 32) {
              inputText += static_cast<char>(event.text.unicode);
              text.setString("<< " + inputText);
            } else if (event.text.unicode == 32) {
              inputText += '_';
              text.setString("<< " + inputText);
            } else if (event.text.unicode == 8) {
              if (!inputText.empty()) {
                inputText.pop_back();
              }
              text.setString("<< " + inputText);
            }
          }
        }
      }
    }
    window.clear();

    if (current_state == WinOrLoose) {
        if(isWordCorrect)
        {
            text.setString("YOU WIN");
            text.setColor(green);
        }
        else
        {
            text.setString("FAILED");
            text.setColor(orange);
        }
      
      text.setPosition(
          sf::Vector2f(window.getSize().x * 0.5, window.getSize().y * 0.5));
      text.setCharacterSize(40);
      sf::FloatRect textRect = text.getLocalBounds();
      text.setOrigin(textRect.left + textRect.width * 0.5f,
                     textRect.top + textRect.height * 0.5f);
      window.draw(text);
    } else if (current_state == SetSecretWord) {
      if (sender) {
        sf::Text tuto_text("", font.font, font.size);
        tuto_text.setPosition(firstMessagePosition);
        tuto_text.setColor(green);
        tuto_text.setString(">> Enter the Word you want to be found: ");
        sf::Vector2f offsetSecretWord = sf::Vector2f(20.0f, 40.0f);
        text.setPosition(firstMessagePosition + offsetSecretWord);
        window.draw(text);
        window.draw(tuto_text);
      } else {
        sf::Text tuto_text("", font.font, font.size);
        tuto_text.setPosition(firstMessagePosition);
        tuto_text.setColor(green);
        tuto_text.setString(">> Waiting For Transmission...");
        window.draw(tuto_text);
      }

      text.setPosition(firstMessagePosition);
    } else {
      bool isFirstColorGreen = isGuesser ? false : true;
      for (int i = 1; i < messagess.size(); i++) {
        sf::Text messageText("", font.font, font.size);
        if (isFirstColorGreen) {
          messageText.setString("  >> " + messagess[i]);
          messageText.setColor(green);
        } else {
          messageText.setString("  << " + messagess[i]);
          messageText.setColor(orange);
        }
        messageText.setPosition(GetPositionStr(i));
        window.draw(messageText);
        isFirstColorGreen = !isFirstColorGreen;
      }
      text.setPosition(GetNewPosition());
      window.draw(text);

      if (isPlayerTurn) {
        sf::Text Info("Your turn ", font.font, font.size);
        Info.setPosition(firstMessagePosition);
        Info.setColor(green);
        window.draw(Info);
      } else {
        sf::Text Info("Waiting transmission ", font.font, font.size);
        Info.setPosition(firstMessagePosition);
        Info.setColor(green);
        window.draw(Info);
      }
    }

    window.display();

    if (current_state == SetSecretWord) {
      // Waiting For the word
      if (!sender) {
        std::cout << "Waiting for Word" << std::endl;
        std::string receivedWord;
        if (socket.receive(packet) == sf::Socket::Done) {
          packet >> receivedWord;
          secretWord = receivedWord;
          std::cout << "Word received from server: " << receivedWord
                    << std::endl;
          current_state = GameState::FindingWord;
        }
      }
    }

    if (current_state == FindingWord) {
      if (isWaitingForTurn) {
        sf::Packet turnPacket;
        if (socket.receive(turnPacket) == sf::Socket::Done) {
          turnPacket >> isPlayerTurn >> wordReceived >> isGuesser >>
              isWordCorrect >> currentTurn;
          std::cout << "received  : " << isPlayerTurn
                    << " Word : " << wordReceived << std::endl;

          sf::Text newMessage("<< " + inputText, font.font, font.size);
          sf::Vector2f newPosition = GetNewPosition();
          newMessage.setPosition(newPosition);
          newMessage.setColor(orange);
          newMessage.setString(wordReceived);
          messages.push_back(newMessage);
          messagess.push_back(wordReceived);
          isWaitingForTurn = false;
        }

        if (!isPlayerTurn) {
          isWaitingForTurn = true;
        }

        if (isWordCorrect || currentTurn >= 20) {
          current_state = WinOrLoose;
        }
      }
    }
  }
}