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

#ifndef TESTMDL_HPP
#define TESTMDL_HPP

#include <glm/glm.hpp>

#include "input.hpp"
#include "light.hpp"
#include "physics.hpp"

class Testmdl_physics final: public Physics
{
public:
    static std::shared_ptr<Testmdl_physics> create();
    void update(Entity & ent, const float dt) override;
};

Entity create_testmdl();

class Testlight_input final: public Input
{
public:
    static std::shared_ptr<Testlight_input> create();
    void update(Entity & ent, const sf::Window & win,
        const float dt) override;
    sigc::signal<void> signal_light_toggled();
    sigc::signal<void> signal_move_toggled();

private:
    sigc::signal<void> _signal_light_toggled;
    sigc::signal<void> _signal_move_toggled;
};

class Testlight_physics final: public Physics
{
public:
    static std::shared_ptr<Testlight_physics> create();
    void update(Entity & ent, const float dt) override;
    void toggle_movement();
private:
    bool _moving = true;
    float _theta = 0.0f;

};

Entity create_testlight();

class Testmonkey_input final: public Input
{
public:
    static std::shared_ptr<Testmonkey_input> create();
    void update(Entity & ent, const sf::Window & win,
        const float dt) override;
    sigc::signal<void> signal_light_toggled();
private:
    sigc::signal<void> _signal_light_toggled;
};

Entity create_testmonkey();

#endif // TESTMDL_HPP
