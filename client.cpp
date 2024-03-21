#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <array>
#include <iostream>
#include <random>
#include <vector>

#include "message.h"

enum class GameState {
  SetSecretWord,
  FindingWord,
  WinOrLoose,
};

[[nodiscard]] int RandomRange(int min, int max) noexcept {
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

[[nodiscard]] std::string ToLower(const std::string& str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

class GameNetwork {
 public:
  sf::TcpSocket socket;

  GameNetwork() {}

  void ConnectToServer() {
    sf::Socket::Status status = socket.connect(HOST_NAME, PORT);
    if (status != sf::Socket::Done) {
      std::cerr << "Could not connect to server ! \n";
    }
  }

  std::string ReceiveSecretWord() {
    sf::Packet packet;
    std::string receivedWord;
    if (socket.receive(packet) == sf::Socket::Done) {
      packet >> receivedWord;

      std::cout << "Word received from server" << std::endl;
    }
    return receivedWord;
  }

  bool ReceiveRole() {
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

  void SendData(std::string userInput, bool isWordCorrect) {
    sf::Packet packetWithWord;
    userInput = ToLower(userInput);
    packetWithWord << userInput << isWordCorrect;
    if (socket.send(packetWithWord) == sf::Socket::Done) {
      std::cout << "Packet Send : " << std::endl;
    }
  }

  // Doit etre coté serveur ne pas partager le secret word
  void SendSecretWord(std::string secretWord) {
    std::cout << "enter Sending" << std::endl;
    sf::Packet packet;
    packet << secretWord;
    if (socket.send(packet) == sf::Socket::Done) {
      std::cout << "Packet Send" << std::endl;
    }
    std::cout << "End " << std::endl;
  }

    //TODO Attention outparameter
  void WaitingForTurn(bool& isPlayerTurn, std::string& wordReceived,
                      bool& isGuesser, bool& isWordCorrect, int& currentTurn) {
    sf::Packet turnPacket;
    if (socket.receive(turnPacket) == sf::Socket::Done) {
      turnPacket >> isPlayerTurn >> wordReceived >> isGuesser >>
          isWordCorrect >> currentTurn;
      std::cout << "received  : " << isPlayerTurn << " Word" << std::endl;
    }
  }
};

// GameClient
class GameLogic {
 public:
  int currentTurn = 0;
  static constexpr int maxTurn = 20;
  bool isGuesser = false;
  bool isSender = false;
  bool isWaitingForTurn = true;
  bool isPlayerTurn = true;
  bool isWordCorrect = false;
  std::string wordReceived;
  std::string secretWord = "secret";
  std::string userInput;
  GameNetwork game_network{};

  GameLogic() {
    isWaitingForTurn = true;
    isPlayerTurn = true;
    isWordCorrect = false;
    currentTurn = 0;
    isGuesser = false;
    isSender = false;
    wordReceived = "";
    currentState = GameState::SetSecretWord;
    secretWord = "secret";
    userInput = "";
  }

  void Init() noexcept {
    game_network.ConnectToServer();
    isSender = game_network.ReceiveRole();
    isGuesser = false;
  }
  void Update(){};

  GameState currentState = GameState::SetSecretWord;
  static constexpr std::array<std::string_view, 6> randomWords = {
      "Oui", "Chien", "SFML_Classic", "reel_", "rouge", "random"};

  void CheckCurrentTry() {
    std::cout << "Message Entered" << std::endl;
    userInput = ToLower(userInput);
    if (userInput == secretWord) {
      std::cout << "Correct!" << std::endl;
      isWordCorrect = true;
    } else {
      std::cout << "Incorrect!" << std::endl;
    }
  }

  void ManageEvent(sf::Event event) {
    if (event.type == sf::Event::KeyPressed) {
      if (event.key.code == sf::Keyboard::Enter) {
        if (currentState == GameState::FindingWord) {
          // if (waiting..) continue
          if (!isWaitingForTurn) {
            if (isGuesser) {
              CheckCurrentTry();
              game_network.SendData(userInput, isWordCorrect);
              isWaitingForTurn = true;
              userInput = "";
              isGuesser = false;
            } else {
              userInput = ToLower(userInput);
              if (userInput == secretWord) {
                int worldSelected = RandomRange(0, 5);
                userInput = randomWords[worldSelected];  // TODO
              }
              isGuesser = true;
              game_network.SendData(userInput, isWordCorrect);
              isWaitingForTurn = true;
              userInput = "";
            }
          }
        }
        if (currentState == GameState::SetSecretWord) {
          secretWord = userInput;
          secretWord = ToLower(secretWord);
          userInput = "";
          if (isSender) {
            game_network.SendSecretWord(secretWord);
            currentState = GameState::FindingWord;
          }
        }
      }

    } else if (event.type == sf::Event::TextEntered) {
      if (!isWaitingForTurn || currentState == GameState::SetSecretWord) {
        if (isPlayerTurn) {
          if (event.text.unicode < 128 && event.text.unicode > 32) {
            userInput += static_cast<char>(event.text.unicode);
          } else if (event.text.unicode == 32) {
            userInput += '_';
          } else if (event.text.unicode == 8) {
            if (!userInput.empty()) {
              userInput.pop_back();
            }
          }
        }
      }
    }
  }
};

class GameView {
  //TODO class color -> constexpr
  const sf::Color green = sf::Color(0.0f, 255.0f, 102.0f);
  const sf::Color orange = sf::Color(255, 175, 64);

  sf::Sprite spritePC;
  sf::Texture texturePC;
  sf::RenderWindow window;
  sf::Text text;
  sf::Font font;

  std::vector<sf::Text> messages;
  std::vector<std::string> messagesHistory;
  sf::Vector2f firstMessagePosition = sf::Vector2f(62.0f, 48.0f);
  float marginYText = 20.0f;

  GameLogic game_logic_{};

  sf::Vector2f GetNewPosition() const noexcept {
    return firstMessagePosition +
           sf::Vector2f(0.0f,
                        static_cast<float>(messages.size()) * marginYText);
  }

  sf::Vector2f GetPositionStr(const int i) const noexcept {
    return firstMessagePosition +
           sf::Vector2f(0.0f, static_cast<float>(i) * marginYText);
  }

 public:
  GameView() {
    texturePC = sf::Texture();
    font = sf::Font();
  }

  void Init() {
    if (!texturePC.loadFromFile("../PCScreen.png")) {
      std::cerr << "Failed Loading Texture\n";
    }
    spritePC.setTexture(texturePC);

    if (!font.loadFromFile("../Anta-Regular.ttf")) {
      std::cerr << "Failed Loading Font\n";
    }
    sf::Text message_send_draw("", font);
    text.setFont(font);
    text.setCharacterSize(20);
    message_send_draw.setPosition(firstMessagePosition);
    text.setPosition(firstMessagePosition);
    text.setString("<< " + game_logic_.userInput);

    window.create({800u, 600u}, "Turn Based Mini GameLogic", sf::Style::None);
    window.setFramerateLimit(60);
    game_logic_.Init();
  }

  void ManageEvent() {
    for (auto event = sf::Event{}; window.pollEvent(event);) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
      game_logic_.ManageEvent(event);
      if (game_logic_.isPlayerTurn) {
        text.setPosition(GetNewPosition());
        text.setString("<< " + game_logic_.userInput);
      }
    }
  }

  void Update() {
    while (window.isOpen()) {
      ManageEvent();
      Render();
    }
  }

  void Render() {
    window.clear();

    if (game_logic_.currentState == GameState::WinOrLoose) {
      if (game_logic_.isWordCorrect) {
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
    } else if (game_logic_.currentState == GameState::SetSecretWord) {
      if (game_logic_.isSender) {
        sf::Text tuto_text("", font);
        tuto_text.setPosition(firstMessagePosition);
        tuto_text.setColor(green);
        tuto_text.setString(">> Enter the word you want P2 to find: ");
        text.setCharacterSize(20);
        sf::Vector2f offsetSecretWord = sf::Vector2f(20.0f, 40.0f);
        text.setPosition(firstMessagePosition + offsetSecretWord);
        window.draw(text);
        window.draw(tuto_text);
      } else {
        sf::Text tuto_text("", font);
        tuto_text.setPosition(firstMessagePosition);
        tuto_text.setColor(green);
        tuto_text.setString(">> Waiting For P1 Transmission...");
        text.setCharacterSize(20);
        window.draw(tuto_text);
      }

      text.setPosition(firstMessagePosition);
    } else {
      text.setCharacterSize(20);
      bool isFirstColorGreen = game_logic_.isGuesser ? false : true;
      for (int i = 1; i < messagesHistory.size(); i++) {
        sf::Text messageText("", font);
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

      if (game_logic_.isPlayerTurn) {
        sf::Text Info("//Send something.. ", font);
        Info.setPosition(firstMessagePosition);
        Info.setColor(green);
        window.draw(Info);
      } else {
        sf::Text Info("//Waiting transmission.. ", font);
        Info.setPosition(firstMessagePosition);
        Info.setColor(green);
        window.draw(Info);
      }
    }

    window.draw(spritePC);
    window.display();

    if (game_logic_.currentState == GameState::SetSecretWord) {
      // Waiting For the word
      if (!game_logic_.isSender) {
        std::cout << "Waiting for Word" << std::endl;
        game_logic_.secretWord = game_logic_.game_network.ReceiveSecretWord();
        game_logic_.currentState = GameState::FindingWord;
      }
    }

    if (game_logic_.currentState == GameState::FindingWord) {
      if (game_logic_.isWaitingForTurn) {
        game_logic_.game_network.WaitingForTurn(
            game_logic_.isPlayerTurn, game_logic_.wordReceived,
            game_logic_.isGuesser, game_logic_.isWordCorrect,
            game_logic_.currentTurn);
        sf::Text newMessage("<< " + game_logic_.userInput, font);
        sf::Vector2f newPosition = GetNewPosition();
        newMessage.setPosition(newPosition);
        newMessage.setColor(orange);
        newMessage.setString(game_logic_.wordReceived);
        messages.push_back(newMessage);
        messagesHistory.push_back(game_logic_.wordReceived);
        game_logic_.isWaitingForTurn = false;

        if (!game_logic_.isPlayerTurn) {
          game_logic_.isWaitingForTurn = true;
        }

        if (game_logic_.isWordCorrect ||
            game_logic_.currentTurn >= game_logic_.maxTurn) {
          game_logic_.currentState = GameState::WinOrLoose;
        }
      }
    }
  }
};

int main() {
  GameView game;
  game.Init();
  game.Update();
}