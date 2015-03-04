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

#include <iostream>
#include <unordered_map>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <glm/glm.hpp>

#include "entity.hpp"

std::shared_ptr<Testmdl_physics> Testmdl_physics::create()
{
    return std::make_shared<Testmdl_physics>();
}

void Testmdl_physics::update(Entity & ent, const float dt)
{
    ent.rotate(dt * 0.5f * M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
}

Entity create_testmdl()
{
    Entity testmdl(Model::create("mdl/weird_cube.dae"),
        std::shared_ptr<Input>(),
        Testmdl_physics::create(),
        std::shared_ptr<Light>());

    testmdl.set_pos(glm::vec3(0.0f, 5.0f, 0.0f));
    testmdl.rotate(M_PI / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    testmdl.rotate(M_PI / 8.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    return testmdl;
}

std::shared_ptr<Testlight_input> Testlight_input::create()
{
    return std::make_shared<Testlight_input>();
}

void Testlight_input::update(Entity & ent, const sf::Window & win,
    const float dt)
{
    static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::P) && !key_lock[sf::Keyboard::P])
    {
        key_lock[sf::Keyboard::P] = true;
        _signal_light_toggled();
    }
    else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::P) && key_lock[sf::Keyboard::P])
    {
        key_lock[sf::Keyboard::P] = false;
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::O) && !key_lock[sf::Keyboard::O])
    {
        key_lock[sf::Keyboard::O] = true;
        _signal_move_toggled();
    }
    else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::O) && key_lock[sf::Keyboard::O])
    {
        key_lock[sf::Keyboard::O] = false;
    }
}

sigc::signal<void> Testlight_input::signal_light_toggled()
{
    return _signal_light_toggled;
}

sigc::signal<void> Testlight_input::signal_move_toggled()
{
    return _signal_move_toggled;
}

Testlight_physics::Testlight_physics(): _moving(true)
{
}

std::shared_ptr<Testlight_physics> Testlight_physics::create()
{
    return std::make_shared<Testlight_physics>();
}

void Testlight_physics::update(Entity & ent, const float dt)
{
    if(!_moving)
        return;

    static float theta = 0.0f;

    ent.set_pos(glm::vec3(10.0f * cosf(theta), 3.0f, 10.0f * sinf(theta)));

    theta += dt * 0.0625f * M_PI;
    if(theta >= 2.0f * M_PI)
    {
        theta -= 2.0f * M_PI;
    }
}

void Testlight_physics::toggle_movement()
{
    _moving = !_moving;
}

Entity create_testlight()
{
    auto model = Model::create("mdl/boring_sphere.dae");
    auto input = Testlight_input::create();
    auto physics = Testlight_physics::create();
    auto light = Point_light::create(true, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f,
        glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.5f, 0.0f);

    Entity ent(model, input, physics, light);

    input->signal_light_toggled().connect(sigc::track_obj([light](){ light->enabled = !light->enabled; }, *light));
    input->signal_move_toggled().connect(sigc::mem_fun(*physics, &Testlight_physics::toggle_movement));

    return ent;
}
