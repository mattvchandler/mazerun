// player.cpp
// player entity

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

#include "player.hpp"

#include <iostream>
#include <stdexcept>
#include <system_error>
#include <unordered_map>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>

#include "entity.hpp"

std::shared_ptr<Player_input> Player_input::create()
{
    return std::make_shared<Player_input>();
}

void Player_input::update(Entity & ent,
    const sf::Window & win, const float dt)
{
    static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;
    static sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(win);

    float mov_scale = 5.0f;
    sf::Vector2i new_mouse_pos = sf::Mouse::getPosition(win);
    // TODO: invalidate signal somehow

    // move forward
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        ent.translate(mov_scale * ent.forward() * dt);
    }

    // move backwards
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        ent.translate(-mov_scale * ent.forward() * dt);
    }

    // move left
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        ent.translate(-mov_scale * ent.right() * dt);
    }

    // move right
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        ent.translate(mov_scale * ent.right() * dt);
    }

    // TODO: replace with jump?
    // move up
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
    {
        ent.translate(mov_scale * glm::vec3(0.0f, 1.0f,  0.0f) * dt);
    }

    // move down
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
    {
        ent.translate(-mov_scale * glm::vec3(0.0f, 1.0f, 0.0f) * dt);
    }

    // toggle spotlight
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::F) && !key_lock[sf::Keyboard::F])
    {
        key_lock[sf::Keyboard::F] = true;
        _signal_spotlight_toggled();
    }
    else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::F) && key_lock[sf::Keyboard::F])
    {
        key_lock[sf::Keyboard::F] = false;
    }

    // Toggle sunlight
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::I) && !key_lock[sf::Keyboard::I])
    {
        key_lock[sf::Keyboard::I] = true;
        _signal_sunlight_toggled();
    }
    else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::I) && key_lock[sf::Keyboard::I])
    {
        key_lock[sf::Keyboard::I] = false;
    }

    // rotate view with mouse click & drag
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        int d_x = new_mouse_pos.x - old_mouse_pos.x;
        int d_y = new_mouse_pos.y - old_mouse_pos.y;

        ent.rotate(0.001f * d_y, ent.right());
        ent.rotate(0.001f * d_x, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    old_mouse_pos = new_mouse_pos;
}

sigc::signal<void> Player_input::signal_spotlight_toggled()
{
    return _signal_spotlight_toggled;
}

sigc::signal<void> Player_input::signal_sunlight_toggled()
{
    return _signal_sunlight_toggled;
}

Entity create_player()
{
    auto model = std::shared_ptr<Model>();
    auto input = Player_input::create();
    auto physics = std::shared_ptr<Physics>();
    auto light = Spot_light::create(true, glm::vec3(1.0f, 1.0f, 0.0f), 1.0f,
        glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), cosf(0.0625f * M_PI), 0.01f, 1.0f, 0.5f, 0.0f);

    Entity player(model, input, physics, light);

    player.set(glm::vec3(0.0f, 1.2f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    input->signal_spotlight_toggled().connect(sigc::track_obj([light](){ light->enabled = !light->enabled; }, *light));

    return player;
}
