// world.cpp
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

// TODO: we've got a lot of state saving & restoring already. after code works, do performance sweep and clean these up when possible

// TODO: why uber-lag when resized? I think it's too much geometry... (but it shouldn't be)

#include "world/world.hpp"

#include <atomic>
#include <chrono>
#include <random>
#include <stdexcept>
#include <system_error>
#include <thread>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <SFML/Audio.hpp>

#include "util/logger.hpp"

thread_local std::mt19937 prng;
thread_local std::random_device rng;

extern std::atomic_bool interrupted; // defined in main.cpp

Glew_init::Glew_init()
{
    if(Glew_init::_initialized)
    {
        Logger_locator::get()(Logger::WARN, "Attempted re-initialization of GLEW");
        throw std::runtime_error("Attempted re-initialization of GLEW");
    }

    Glew_init::_initialized = true;
    Logger_locator::get()(Logger::TRACE, std::string("GLEW initialized. OpenGL version: ") + (const char *)glGetString(GL_VERSION));


    GLenum glew_status = glewInit();
    if(glew_status != GLEW_OK)
    {
        Logger_locator::get()(Logger::ERROR, std::string("Error loading GLEW:") + (const char *)glewGetErrorString(glew_status));
        throw std::runtime_error(std::string("Error loading GLEW: ") + (const char *)glewGetErrorString(glew_status));
    }

    // Check for minimum supported OpenGL version
    if(!GLEW_VERSION_3_0)
    {
        Logger_locator::get()(Logger::ERROR, std::string("Error OpenGL version (") + (const char *)glGetString(GL_VERSION) + ") is too low.\nVersion 3.0+ required");
        throw std::runtime_error(std::string("Error OpenGL version (") + (const char *)glGetString(GL_VERSION) + ") is too low.\nVersion 3.0+ required");
    }

    // TODO: display context settings
}

bool Glew_init::_initialized = false;

// TODO: picking. Should we always do a pick pass, or make a 'pick' method?
    // need: picking shader, target ent ptr
void World::resize()
{
    // projection matrix setup
    glViewport(0, 0, _win.getSize().x, _win.getSize().y);
    _proj = glm::perspective((float)M_PI / 6.0f,
        (float)_win.getSize().x / (float)_win.getSize().y, 0.1f, 1000.0f);
    // TODO: request redraw
    // TODO: resize framebuffer attachments
}

void World::game_loop()
{
    // due to quirk of SFML event handling needs to be in main thread
    _win.setActive(false); // decativate rendering context for main thread

    // rendering, physics, input, etc to be handled in this thread
    std::thread main_loop_t(&World::main_loop, this);
    std::thread message_loop_t(&World::message_loop, this);

    event_loop(); // handle events

    main_loop_t.join();
    message_loop_t.join();
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
                _do_resize = true;
                break;
            case sf::Event::KeyPressed:
                Message_locator::get().queue_event<sf::Keyboard::Key>("key_down", ev.key.code);
            default:
                break;
            }
        }
        if(!_running || interrupted)
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
    sf::Clock dt_clk;
    prng.seed(rng());
    _win.setActive(true); // set render context active for this thread
    while(true)
    {
        float dt = dt_clk.restart().asSeconds();

        _lock.lock();
        if(_do_resize)
        {
            resize();
            _do_resize = false;
        }

        if(!_running || interrupted)
        {
            _lock.unlock();
            break;
        }
        if(_focused)
        {
            for(auto & ent: _ents)
            {
                auto input = ent.input();
                if(input)
                {
                    input->update(ent, _win, dt);
                }
            }
        }

        for(auto & ent: _ents)
        {
            auto physics = ent.physics();
            if(physics)
            {
                physics->update(ent, dt);
            }
        }

        for(auto & ent: _ents)
        {
            auto audio = ent.audio();
            if(audio)
            {
                audio->update(ent);
            }
        }

        // set audio listener
        glm::vec3 cam_pos = _cam->pos();
        glm::vec3 cam_forward = _cam->forward();
        glm::vec3 cam_up = _cam->up();
        sf::Listener::setPosition(cam_pos.x, cam_pos.y, cam_pos.z);
        sf::Listener::setDirection(cam_forward.x, cam_forward.y, cam_forward.z);
        sf::Listener::setUpVector(cam_up.x, cam_up.y, cam_up.z);

        // TODO should we make more threads for input, physics, messages, etc?
        // TODO: AI
            // State machine: Idle, attack, flee, travel
                // some states have a target entity (attack this, or flee from this or travel to that
            // goal: overrides state:
                // defend: keep target in sight at all times, attack any foe in sight, prioritize by dist to target
                    // given entity (can be invisible marker node)
            // relationship db.
                // if ent not in db, neutral, otherwise positive score is friendly, negative is hostile
                // attacking will lower relationship. giving items will increase?
                // relationships in/decrease toward defalt value with time
            // pathfinding
                // subdivide mazegrid && Dijkstra's
        draw();
        _lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }
}

void World::message_loop()
{
    prng.seed(rng());
    while(true)
    {
        if(!_running || interrupted)
        {
            break;
        }
        if(!Message_locator::get().queue_empty())
        {
            _lock.lock();
            Message_locator::get().process_events();
            _lock.unlock();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
