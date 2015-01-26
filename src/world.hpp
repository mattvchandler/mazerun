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

#ifndef WORLD_HPP
#define WORLD_HPP

#include <mutex>

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include "light.hpp"
#include "player.hpp"
#include "skybox.hpp"
#include "shader_prog.hpp"
#include "walls.hpp"

class World // TODO: make this a singleton?
{
public:
    World(); // TODO should we take default args?
    bool init();
    void draw();
    void resize();
    void game_loop();
protected:
    void event_loop();
    void main_loop();

    bool _running;
    bool _focused;
    bool _do_resize; // TODO: find a better way (prob. messages/signals)

    std::mutex _lock; // TODO more descriptive name

    sf::Window _win;

    glm::mat4 _proj;

    Skybox _skybox;
    Dir_light _sunlight;
    Player _player;
    Walls _walls;
    Floor _floor;
    Shader_prog _ent_shader;
    // TODO: entity table
};

#endif // WORLD_HPP
