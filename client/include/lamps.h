#pragma once

#include <SFML/Graphics.hpp>
#include <string>
 
class Lamp {
public:
    Lamp(sf::Vector2f position, float radius, sf::Color color)
        : m_position(position), m_radius(radius), m_color(color) {
        m_circle.setRadius(radius);
        m_circle.setFillColor(color);
        m_circle.setPosition(position);
        m_circle.setOrigin(radius, radius);
    }

    virtual ~Lamp() {}

    void draw(sf::RenderWindow &window) {
        window.draw(m_circle);
    }

    void changeColor(const sf::Color &color) {
        m_color = color;
        m_circle.setFillColor(color);
    }

private:
    sf::Vector2f   m_position;
    float          m_radius;
    sf::Color      m_color;
    sf::CircleShape m_circle;
};

void lamp_create(std::vector<Lamp> &lamps);