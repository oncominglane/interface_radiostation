#include "lamps.h"
#include "config.h"

// Функция для создания лампочек
void lamp_create(std::vector<Lamp> &lamps) {
    float lampRadius = 50.0f;
    float startX = resolution_x - 100.0f;  // Правый край экрана
    float startY = 100.0f;
    float lampOffset = 150.0f;

    lamps.emplace_back(sf::Vector2f(startX, startY), lampRadius, sf::Color::Black);
    lamps.emplace_back(sf::Vector2f(startX, startY + lampOffset), lampRadius, sf::Color::Black);
    lamps.emplace_back(sf::Vector2f(startX, startY + 2 * lampOffset), lampRadius, sf::Color::Black);
}