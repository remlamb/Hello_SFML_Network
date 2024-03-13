#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <array>
#include <iostream>
#include <random>
#include <vector>

#include "message.h"

enum GameState {
  SetSecretWord,
  FindingWord,
  WinOrLoose,
};

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

  Font() : size(20) { LoadFont(); }
  void SetSize(int size) { this->size = size; }
};

static const sf::Color green(0.0f, 255.0f, 102.0f);
static const sf::Color orange(255, 175, 64);

[[nodiscard]] int randomRange(int min, int max) noexcept {
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(min, max);
  return distr(eng);
}

[[nodiscard]] bool CompareString(const std::string& str1,
                                 const std::string& str2) noexcept {
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

class Game {
 public:
  bool isWaitingForTurn = true;
  bool isPlayerTurn = true;
  bool isWordCorrect = false;
  int currentTurn = 0;
  bool isGuesser = false;
  bool sender = false;
  std::string wordReceived = "";
  sf::TcpSocket socket;
  GameState current_state = SetSecretWord;
  std::string secretWord = "secret";
  std::string inputText = "";
  std::array<std::string, 6> randomWord = {"Oui",     "Chien", "SFML_Classic",
                                           "Galaxy_", "rouge", "random"};
  sf::Sprite spritePC;
  sf::Texture texturePC;
  sf::Packet packet;
  sf::RenderWindow window;
  sf::Text text;
  Font font;

  std::vector<sf::Text> messages;
  std::vector<std::string> messagesHistory;
  sf::Vector2f firstMessagePosition = sf::Vector2f(62.0f, 48.0f);
  float marginYText = 20.0f;

  Game() {
    isWaitingForTurn = true;
    isPlayerTurn = true;
    isWordCorrect = false;
    currentTurn = 0;
    isGuesser = false;
    sender = false;
    wordReceived = "";
    current_state = SetSecretWord;
    secretWord = "secret";
    inputText = "";
    texturePC = sf::Texture();
    font = Font();
  }

  sf::Vector2f GetNewPosition() {
    return firstMessagePosition +
           sf::Vector2f(0.0f,
                        static_cast<float>(messages.size()) * marginYText);
  }

  sf::Vector2f GetPositionStr(int i) {
    return firstMessagePosition +
           sf::Vector2f(0.0f, static_cast<float>(i) * marginYText);
  }

  void ConnectToServer() {
    sf::Socket::Status status = socket.connect(HOST_NAME, PORT);
    if (status != sf::Socket::Done) {
      // error
      std::cerr << "Could not connect to server ! \n";
      // return EXIT_FAILURE;
    }
  }
  void ReceiveRole() {
    sf::Packet packet;
    if (socket.receive(packet) != sf::Socket::Done) {
      std::cerr << "Failed to receive sender flag from server" << std::endl;
      // return EXIT_FAILURE;
    }
    packet >> sender;
    std::cout << "Received sender flag from server: " << std::boolalpha
              << sender << std::endl;
  }

  void CheckCurrentTry() {
    std::cout << "Message Entered" << std::endl;
    if (inputText == secretWord) {
      std::cout << "Correct!" << std::endl;
      isWordCorrect = true;
      // current_state = GameState::WinOrLoose;
    } else {
      std::cout << "Incorrect!" << std::endl;
    }
  }

  void SendData() {
    sf::Packet packetWithWord;
    packetWithWord << inputText << isWordCorrect;
    if (socket.send(packetWithWord) == sf::Socket::Done) {
      std::cout << "Packet Send : " << std::endl;
      isWaitingForTurn = true;
    }
  }
  void SendSecretWord() {
    std::cout << "enter Sending" << std::endl;
    sf::Packet packet;
    packet << secretWord;
    if (socket.send(packet) == sf::Socket::Done) {
      std::cout << "Packet Send" << std::endl;
      current_state = GameState::FindingWord;
    }
    std::cout << "End " << std::endl;
  }

  void SetUp() {
    if (!texturePC.loadFromFile("../PCScreen.png")) {
      std::cerr << "Failed Loading Texture\n";
    }
    spritePC.setTexture(texturePC);

    ConnectToServer();
    ReceiveRole();

    sf::Text message_send_draw("", font.font, font.size);
    isGuesser = false;
    text.setFont(font.font);
    text.setCharacterSize(font.size);
    message_send_draw.setPosition(firstMessagePosition);
    text.setPosition(firstMessagePosition);
    text.setString("<< " + inputText);

    window.create({800u, 600u}, "Turn Based Mini Game", sf::Style::Close);
    window.setFramerateLimit(60);
  }

  void ManageEvent() {
    for (auto event = sf::Event{}; window.pollEvent(event);) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
          if (current_state == GameState::FindingWord) {
            if (!isWaitingForTurn) {
              if (isGuesser) {
                CheckCurrentTry();

                sf::Text newMessage(">> " + inputText, font.font, font.size);
                sf::Vector2f newPosition = GetNewPosition();
                newMessage.setPosition(newPosition);
                newMessage.setColor(green);
                text.setPosition(GetNewPosition());

                SendData();
                inputText = "";
                text.setString(">> " + inputText);
                isGuesser = false;
              } else {
                if (inputText == secretWord) {
                  int worldSelected = randomRange(0, 5);
                  inputText = randomWord[worldSelected];
                  // inputText = "random";  // TODO
                }
                sf::Text newMessage("<< " + inputText, font.font, font.size);
                sf::Vector2f newPosition = GetNewPosition();
                newMessage.setPosition(newPosition);
                newMessage.setColor(orange);
                text.setPosition(GetNewPosition());

                isGuesser = true;

                SendData();
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
              SendSecretWord();
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
  }
  void Render() {
    window.clear();

    if (current_state == WinOrLoose) {
      if (isWordCorrect) {
        text.setString("Good Job");
        text.setColor(green);
      } else {
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
        tuto_text.setString(">> Enter the word you want P2 to find: ");
        sf::Vector2f offsetSecretWord = sf::Vector2f(20.0f, 40.0f);
        text.setPosition(firstMessagePosition + offsetSecretWord);
        window.draw(text);
        window.draw(tuto_text);
      } else {
        sf::Text tuto_text("", font.font, font.size);
        tuto_text.setPosition(firstMessagePosition);
        tuto_text.setColor(green);
        tuto_text.setString(">> Waiting For P1 Transmission...");
        window.draw(tuto_text);
      }

      text.setPosition(firstMessagePosition);
    } else {
      bool isFirstColorGreen = isGuesser ? false : true;
      for (int i = 1; i < messagesHistory.size(); i++) {
        sf::Text messageText("", font.font, font.size);
        if (isFirstColorGreen) {
          messageText.setString("  >> " + messagesHistory[i]);
          messageText.setColor(green);
        } else {
          messageText.setString("  << " + messagesHistory[i]);
          messageText.setColor(orange);
        }
        messageText.setPosition(GetPositionStr(i));
        window.draw(messageText);
        isFirstColorGreen = !isFirstColorGreen;
      }
      text.setPosition(GetNewPosition());
      window.draw(text);

      if (isPlayerTurn) {
        sf::Text Info("//Send something.. ", font.font, font.size);
        Info.setPosition(firstMessagePosition);
        Info.setColor(green);
        window.draw(Info);
      } else {
        sf::Text Info("//Waiting transmission.. ", font.font, font.size);
        Info.setPosition(firstMessagePosition);
        Info.setColor(green);
        window.draw(Info);
      }
    }

    window.draw(spritePC);
    window.display();

    if (current_state == SetSecretWord) {
      // Waiting For the word
      if (!sender) {
        std::cout << "Waiting for Word" << std::endl;
        std::string receivedWord;
        if (socket.receive(packet) == sf::Socket::Done) {
          packet >> receivedWord;
          secretWord = receivedWord;
          std::cout << "Word received from server" << std::endl;
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
          std::cout << "received  : " << isPlayerTurn << " Word" << std::endl;

          sf::Text newMessage("<< " + inputText, font.font, font.size);
          sf::Vector2f newPosition = GetNewPosition();
          newMessage.setPosition(newPosition);
          newMessage.setColor(orange);
          newMessage.setString(wordReceived);
          messages.push_back(newMessage);
          messagesHistory.push_back(wordReceived);
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

  void Update() {
    while (window.isOpen()) {
      ManageEvent();
      Render();
    }
  }
};

int main() {
  Game game;
  game.SetUp();
  game.Update();
}