#pragma once

#include <iostream>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "config.h"

class ButtonCircle {  // TODO namespaces
public:
    ButtonCircle(const sf::Vector2f &position, const float radius, const std::string &texturePath, const std::string &command,
                 const std::string &text):
            m_command(command), m_position(position), m_radius(radius) {
        m_texture.loadFromFile(texturePath);

        m_font.loadFromFile("assets/troika.otf");

        m_buttonText.setFont(m_font);
        m_buttonText.setString(text);
        m_buttonText.setCharacterSize(__button_text_size);
        m_buttonText.setFillColor(sf::Color::White);
        m_buttonText.setPosition(position.x, position.y - radius - static_cast<float>(__text_offset) * 1.5);

        m_circle.setRadius(radius);
        m_circle.setPosition(position);
        m_circle.setTexture(&m_texture);

        m_circle.setOrigin(radius, radius);
    }

    void draw(sf::RenderWindow &window) {
        window.draw(m_circle);
        window.draw(m_buttonText);
    }

    bool is_mouse_over(sf::Vector2f &mouse_pos) {
        float distance = std::sqrt(std::pow(mouse_pos.x - m_position.x, 2) + std::pow(mouse_pos.y - m_position.y, 2));

        if (distance <= m_radius)
            return true;

        return false;
    }

    void change_color(const sf::Color &color) { m_circle.setFillColor(color); }

    std::string m_command;

private:
    sf::Vector2f    m_position;
    sf::Text        m_buttonText;
    sf::Font        m_font;
    sf::Texture     m_texture;
    sf::CircleShape m_circle;
    float           m_radius;
};

void buttons_create(std::vector<ButtonCircle *> &buttons);