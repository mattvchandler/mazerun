// world.hpp
// world object

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

// TODO: window is owned here. can we ensure that all openGL objects are constructed after the window (so that GL context is valid)
//       If so, move inits to ctors

// TODO: we've got a lot of state saving & restoring already. after code works, do performance sweep and clean these up when possible

// TODO: why uber-lag when resized?

#include <iostream>
#include <random>
#include <thread>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "world.hpp"
#include "gl_helpers.hpp" // TODO: move w/ walls decl

thread_local std::mt19937 prng;
thread_local std::random_device rng;

World::World(): _running(true), _focused(true), _do_resize(false),
    _win(sf::VideoMode(800, 600), "mazerun", sf::Style::Default, sf::ContextSettings(24, 8, 8, 3, 0))
{
}

bool World::init()
{
    // TODO: loading screen
    if(glewInit() != GLEW_OK)
    {
        std::cerr<<"Error loading glew"<<std::endl;
        return false; // TODO: exception? at least be consistent w/ other inits
    }

    _win.setKeyRepeatEnabled(false);
    _win.setFramerateLimit(60);
    // TODO _win.setIcon

    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.0f, 1.0f);
    glLineWidth(5.0f);

    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendColor(1.0f, 1.0f, 1.0f, 0.1f);
    glEnable(GL_BLEND);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    resize();

    // set camera initial position / orientation
    _player.set(glm::vec3(1.0f, -10.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));


    _skybox.init();
    // _player.init();
    _walls.init();
    return true;
}

void World::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _skybox.draw(_player, _proj);
    _walls.draw(_player, _proj);
    _win.display();
}

void World::resize()
{
    // projection matrix setup
    glViewport(0, 0, _win.getSize().x, _win.getSize().y);
    _proj = glm::perspective((float)M_PI / 6.0f,
        (float)_win.getSize().x / (float)_win.getSize().y, 0.1f, 1000.0f);
    // TODO: request redraw
}

void World::game_loop()
{
    // due to quirk of SFML event handling needs to be in main thread
    _win.setActive(false); // decativate rendering context for main thread

    // rendering, physics, input, etc to be handled in this thread
    std::thread main_loop_t(&World::main_loop, this);

    event_loop(); // handle events

    main_loop_t.join();
    _win.close();
}

void World::event_loop()
{
    prng.seed(rng());
    while(true)
    {
        // handle events
        sf::Event ev;
        if(_win.waitEvent(ev)) // blocking call
        {
            _lock.lock();
            // TODO: have events trigger signals that listeners can recieve?
            switch(ev.type)
            {
                case sf::Event::Closed:
                    _running = false;
                    break;
                case sf::Event::GainedFocus:
                    _focused = true;
                    break;
                case sf::Event::LostFocus:
                    _focused = false;
                    break;
                case sf::Event::Resized:
                    resize();
                    _do_resize = true;
                    break;
                default:
                    break;
            }
        }
        if(!_running)
        {
            _lock.unlock();
            break;
        }
        _lock.unlock();
    }
}

// runs in a new thread
void World::main_loop()
{
    prng.seed(rng());
    _win.setActive(true); // set render context active for this thread
    while(true)
    {
        _lock.lock();
        if(_do_resize)
        {
            resize();
            _do_resize = false;
        }

        if(!_running)
        {
            _lock.unlock();
            break;
        }
        // std::cerr<<"("<<_player.pos().x<<","<<_player.pos().y<<","<<_player.pos().z<<")"<<std::endl;
        if(_focused)
        {
            _player.handle_input(_win, 1.0f);
        }
        // TODO should we make more threads for input, physics, messages, etc?
        // TODO: physics / AI updates
        draw();
        _lock.unlock();
        // TODO sleep? (may not be needed if we use SFML's frame limit)
    }
}
