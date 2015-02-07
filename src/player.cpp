// player.cpp
// player class

// Copyright 2015 Matthew Chandler

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <stdexcept>
#include <system_error>
#include <unordered_map>

#include <glm/glm.hpp>

#include <SFML/OpenGL.hpp>

#include "player.hpp"

void Player::handle_input(const sf::Window & win, const float dt)
{
    static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;
    static sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(win);

    float mov_scale = 0.1f;
    sf::Vector2i new_mouse_pos = sf::Mouse::getPosition(win);
    // TODO: invalidate signal somehow

    // move forward
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        translate(mov_scale * forward() * dt);
    }

    // move backwards
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        translate(-mov_scale * forward() * dt);
    }

    // move left
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        translate(-mov_scale * right() * dt);
    }

    // move right
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        translate(mov_scale * right() * dt);
    }

    // TODO: replace with jump?
    // move up
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
    {
        translate(mov_scale * glm::vec3(0.0f, 1.0f,  0.0f) * dt);
    }

    // move down
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
    {
        translate(-mov_scale * glm::vec3(0.0f, 1.0f, 0.0f) * dt);
    }

    // rotate view with mouse click & drag
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        int d_x = new_mouse_pos.x - old_mouse_pos.x;
        int d_y = new_mouse_pos.y - old_mouse_pos.y;

        rotate(0.001f * d_y, right());
        rotate(0.001f * d_x, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    old_mouse_pos = new_mouse_pos;
}
