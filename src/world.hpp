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

#include <memory>
#include <mutex>

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <sigc++/sigc++.h>

#include "entity.hpp"
#include "light.hpp"
#include "skybox.hpp"
#include "shader_prog.hpp"
#include "walls.hpp"

class Glew_init final: public sf::NonCopyable
{
public:
    Glew_init();
private:
    static bool _initialized;
};

class World final // TODO: make this a singleton?
{
private:
    sf::Window _win; // we need the OpenGL context to be created before anything else
    Glew_init _glew_init; // needs to run after OpenGL, but before anything else (kind of a hack)
public:
    World(); // TODO should we take default args?
    void draw();
    void resize();
    void game_loop();

private:
    void event_loop();
    void main_loop();

    bool _running;
    bool _focused;
    bool _do_resize; // TODO: find a better way (prob. messages/signals)

    std::mutex _lock; // TODO more descriptive name

    glm::mat4 _proj;

    Skybox _skybox;
    Dir_light _sunlight;
    Walls _walls;
    Floor _floor;
    Shader_prog _ent_shader;

    // entity tables
    std::vector<Entity> _ents; // TODO: list might be better
    // std::vector<Model> _model_components;
    // std::vector<Input> _input_components;
    // std::vector<Physics> _physics_components;

    Entity & _cam;
    Entity & _player;
};

#endif // WORLD_HPP
