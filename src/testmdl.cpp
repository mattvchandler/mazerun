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

#include "entity.hpp"

Testmdl_physics::Testmdl_physics()
{
}

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

Testlight_input::Testlight_input(): _light_on(true)
{
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
        _light_on = !_light_on;

        if(_light_on)
        {
            ent.light()->color = glm::vec3(0.0f, 1.0f, 1.0f);
        }
        else
        {
            ent.light()->color = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        key_lock[sf::Keyboard::P] = true;
    }
    else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::P) && key_lock[sf::Keyboard::P])
    {
        key_lock[sf::Keyboard::P] = false;
    }
}

Testlight_physics::Testlight_physics()
{
}

std::shared_ptr<Testlight_physics> Testlight_physics::create()
{
    return std::make_shared<Testlight_physics>();
}

void Testlight_physics::update(Entity & ent, const float dt)
{
    static float theta = 0.0f;

    ent.set_pos(glm::vec3(10.0f * cosf(theta), 3.0f, 10.0f * sinf(theta)));

    theta += dt;
    if(theta >= 2.0f * M_PI)
    {
        theta -= 2.0f * M_PI;
    }
}

Entity create_testlight()
{
     return Entity(std::shared_ptr<Model>(),
        Testlight_input::create(),
        Testlight_physics::create(),
        Point_light::create(glm::vec3(0.0f, 1.0f, 1.0f), 1.0f,
            glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.5f, 0.0f));
}
