#pragma once

#include <string>
#include <SFML/Network.hpp>

struct Message
{
    std::string content;
};

sf::Packet& operator <<(sf::Packet& packet, const Message& message)
{
    return packet << message.content;
}

sf::Packet& operator >>(sf::Packet& packet, Message& message)
{
    return packet >> message.content;
}