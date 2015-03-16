// testmdl.cpp
// test entity

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

#include "testmdl.hpp"

#include <unordered_map>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <glm/glm.hpp>

#include "config.hpp"
#include "entity.hpp"
#include "logger.hpp"

std::shared_ptr<Testmdl_physics> Testmdl_physics::create()
{
    return std::make_shared<Testmdl_physics>();
}

void Testmdl_physics::update(Entity & ent, const float dt)
{
    ent.rotate_world(dt * 0.5f * M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
}

Entity create_testmdl()
{
    Entity testmdl(Model::create(check_in_pwd("mdl/weird_cube.dae")),
        std::shared_ptr<Input>(),
        Testmdl_physics::create(),
        std::shared_ptr<Light>());

    testmdl.set_pos(glm::vec3(0.0f, 5.0f, 0.0f));
    testmdl.rotate_world(M_PI / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    testmdl.rotate_world(M_PI / 8.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    return testmdl;
}

std::shared_ptr<Testlight_input> Testlight_input::create()
{
    return std::make_shared<Testlight_input>();
}

void Testlight_input::update(Entity & ent, const sf::Window & win,
    const float dt)
{
}

void Testlight_input::key_down(const Message::Packet & pkt)
{
    const sf::Keyboard::Key & key = Message::get_packet<sf::Keyboard::Key>(pkt);
    if(key == sf::Keyboard::P)
        _signal_light_toggled();
    else if(key == sf::Keyboard::O)
        _signal_move_toggled();
}

sigc::signal<void> Testlight_input::signal_light_toggled()
{
    return _signal_light_toggled;
}

sigc::signal<void> Testlight_input::signal_move_toggled()
{
    return _signal_move_toggled;
}

std::shared_ptr<Testlight_physics> Testlight_physics::create()
{
    return std::make_shared<Testlight_physics>();
}

void Testlight_physics::update(Entity & ent, const float dt)
{
    if(!_moving)
        return;

    ent.set_pos(glm::vec3(10.0f * std::cos(_theta), 3.0f, 10.0f * std::sin(_theta)));

    _theta += dt * 0.0625f * M_PI;
    if(_theta >= 2.0f * M_PI)
    {
        _theta -= 2.0f * M_PI;
    }
}

void Testlight_physics::toggle_movement()
{
    _moving = !_moving;
}

Entity create_testlight()
{
    auto model = Model::create(check_in_pwd("mdl/boring_sphere.dae"));
    auto input = Testlight_input::create();
    auto physics = Testlight_physics::create();
    auto light = Point_light::create(true, glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.5f, 0.0f);

    Entity ent(model, input, physics, light);

    input->signal_light_toggled().connect(sigc::track_obj([light](){ light->enabled = !light->enabled; }, *light));
    input->signal_move_toggled().connect(sigc::mem_fun(*physics, &Testlight_physics::toggle_movement));
    Message::add_callback("key_down", sigc::mem_fun(*input, &Testlight_input::key_down));

    return ent;
}

std::shared_ptr<Testmonkey_input> Testmonkey_input::create()
{
    return std::make_shared<Testmonkey_input>();
}

void Testmonkey_input::update(Entity & ent, const sf::Window & win,
    const float dt)
{
}

void Testmonkey_input::key_down(const Message::Packet & pkt)
{
    const sf::Keyboard::Key & key = Message::get_packet<sf::Keyboard::Key>(pkt);
    if(key == sf::Keyboard::U)
        _signal_light_toggled();
}

sigc::signal<void> Testmonkey_input::signal_light_toggled()
{
    return _signal_light_toggled;
}

Entity create_testmonkey()
{
    auto model = Model::create(check_in_pwd("mdl/monkey.dae"));
    auto input = Testmonkey_input::create();
    auto physics = std::shared_ptr<Physics>();
    auto light = Spot_light::create(true, glm::vec3(0.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f), std::cos(10.0f * M_PI / 180.0f), 90.0f,
        0.0f, 0.1f, 0.0f);

    Entity ent(model, input, physics, light);

    ent.set_pos(glm::vec3(-10.0f, 10.0f, -10.0f));
    ent.rotate_world(0.25f * M_PI, ent.up());
    ent.rotate_world(0.25f * M_PI, ent.right());

    input->signal_light_toggled().connect(sigc::track_obj([light](){ light->enabled = !light->enabled; }, *light));
    Message::add_callback("key_down", sigc::mem_fun(*input, &Testmonkey_input::key_down));

    return ent;
}
