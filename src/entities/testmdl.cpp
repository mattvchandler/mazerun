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

#include "entities/testmdl.hpp"

#include <unordered_map>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <glm/glm.hpp>

#include "config.hpp"
#include "util/logger.hpp"
#include "world/entity.hpp"

void Testmdl_physics::update(Entity & ent, const float dt)
{
    ent.rotate_world(dt * 0.5f * M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
}

Entity create_testmdl()
{
    Entity testmdl(Model::create(check_in_pwd("mdl/weird_cube.dae"), true),
        nullptr, // input
        new Testmdl_physics,
        nullptr, // light
        nullptr); // audio

    testmdl.set_pos(glm::vec3(0.0f, 5.0f, 0.0f));
    testmdl.rotate_world(M_PI / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    testmdl.rotate_world(M_PI / 8.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    return testmdl;
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

void Testlight_physics::update(Entity & ent, const float dt)
{
    if(!_moving)
        return;

    ent.set_pos(glm::vec3(8.0f * std::cos(_theta), 3.0f, 8.0f * std::sin(_theta)));

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
    Model * model = Model::create(check_in_pwd("mdl/boring_sphere.dae"), true);
    Testlight_input * input = new Testlight_input;
    Testlight_physics * physics = new Testlight_physics;
    Point_light * light = new Point_light(true, glm::vec3(1.0f, 0.0f, 0.0f), true,
        glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.5f, 0.0f);
    Audio * audio = new Audio(glm::vec3(0.0f));

    Entity ent(
        model,
        input,
        physics,
        light,
        audio);

    input->signal_light_toggled().connect(sigc::track_obj(sigc::track_obj([light, audio]()
    {
        light->enabled = !light->enabled;
        audio->play_sound(check_in_pwd("sound/clickclick.ogg"));
    }, *light), *audio));
    input->signal_move_toggled().connect(sigc::mem_fun(*physics, &Testlight_physics::toggle_movement));

    Message_locator::get().add_callback("key_down", sigc::mem_fun(*input, &Testlight_input::key_down));

    Jukebox_locator::get().preload_sound(check_in_pwd("sound/clickclick.ogg"));

    return ent;
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
    Model * model = Model::create(check_in_pwd("mdl/monkey.dae"), true);
    Testmonkey_input * input = new Testmonkey_input;
    Spot_light * light = new Spot_light(true, glm::vec3(0.0f, 1.0f, 1.0f), true,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f), std::cos(10.0f * M_PI / 180.0f), 90.0f,
        0.0f, 0.1f, 0.0f);
    Audio * audio = new Audio(glm::vec3(0.0f));

    Entity ent(
        model,
        input,
        nullptr, // physics
        light,
        audio);

    ent.set_pos(glm::vec3(-10.0f, 10.0f, -10.0f));
    ent.rotate_world(0.25f * M_PI, ent.up());
    ent.rotate_world(0.25f * M_PI, ent.right());

    audio->set_pos(ent.pos());

    input->signal_light_toggled().connect(sigc::track_obj(sigc::track_obj([light, audio]()
    {
        light->enabled = !light->enabled;
        audio->play_sound(check_in_pwd("sound/tack.ogg"));
    }, *light), *audio));
    Message_locator::get().add_callback("key_down", sigc::mem_fun(*input, &Testmonkey_input::key_down));

    Jukebox_locator::get().preload_sound(check_in_pwd("sound/tack.ogg"));
    audio->play_music(check_in_pwd("sound/Monkeys_Spinning_Monkeys.ogg"), 100.0f, true);

    return ent;
}

void Testdoughnut_input::update(Entity & ent, const sf::Window & win,
    const float dt)
{
}

void Testdoughnut_input::key_down(const Message::Packet & pkt)
{
    const sf::Keyboard::Key & key = Message::get_packet<sf::Keyboard::Key>(pkt);
    if(key == sf::Keyboard::LBracket)
        _signal_move_toggled();
    else if(key == sf::Keyboard::RBracket)
        _signal_rot_toggled();
}

sigc::signal<void> Testdoughnut_input::signal_move_toggled()
{
    return _signal_move_toggled;
}

sigc::signal<void> Testdoughnut_input::signal_rot_toggled()
{
    return _signal_rot_toggled;
}

void Testdoughnut_physics::update(Entity & ent, const float dt)
{
    if(_moving)
    {
        ent.set_pos(glm::vec3(4.0f * std::cos(-_theta), std::cos(4.0f * _theta) + 3.0f, 4.0f * std::sin(-_theta)));

        _theta += dt * 0.0625f * M_PI;
        if(_theta >= 2.0f * M_PI)
        {
            _theta -= 2.0f * M_PI;
        }
    }

    if(_rotating)
    {
        ent.rotate_world(dt * -0.25f * M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void Testdoughnut_physics::toggle_movement()
{
    _moving = !_moving;
}

void Testdoughnut_physics::toggle_rotation()
{
    _rotating = !_rotating;
}

Entity create_testdoughnut()
{
    Model * model = Model::create(check_in_pwd("mdl/mirrors.dae"), true);
    Testdoughnut_input * input = new Testdoughnut_input;
    Testdoughnut_physics * physics = new Testdoughnut_physics;

    Entity ent(
        model,
        input,
        physics,
        nullptr, // light
        nullptr); // audio

    input->signal_move_toggled().connect(sigc::mem_fun(*physics, &Testdoughnut_physics::toggle_movement));
    input->signal_rot_toggled().connect(sigc::mem_fun(*physics, &Testdoughnut_physics::toggle_rotation));

    Message_locator::get().add_callback("key_down", sigc::mem_fun(*input, &Testdoughnut_input::key_down));

    return ent;
}
