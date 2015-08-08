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

#include "components/light.hpp"
#include "entities/walls.hpp"
#include "opengl/shader_prog.hpp"
#include "util/font.hpp"
#include "util/message.hpp"
#include "util/static_text.hpp"
#include "world/entity.hpp"
#include "world/quad.hpp"
#include "world/skybox.hpp"

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
    void message_loop();

    bool _running;
    bool _focused;
    bool _do_resize;

    std::mutex _lock; // TODO more descriptive name

    glm::mat4 _proj;

    Skybox _skybox;
    Dir_light _sunlight;

    Shader_prog _ent_prepass;
    Shader_prog _point_light_prog;
    Shader_prog _point_light_shadow_prog;
    Shader_prog _spot_light_prog;
    Shader_prog _spot_light_shadow_prog;
    Shader_prog _dir_light_prog;
    Shader_prog _dir_light_shadow_prog;
    Shader_prog _point_shadow_prog;
    Shader_prog _spot_dir_shadow_prog;
    Shader_prog _set_depth_prog;
    Shader_prog _ent_shader;
    Shader_prog _fullscreen_tex;

    FBO _g_fbo;
    FBO _lighting_fbo;
    FBO _point_shadow_fbo;
    FBO _spot_dir_shadow_fbo;

    std::unique_ptr<Texture_2D> _g_fbo_norm_shininess_tex;
    std::unique_ptr<Texture_2D> _g_fbo_depth_tex;
    std::unique_ptr<Texture_2D> _diffuse_fbo_tex;
    std::unique_ptr<Texture_2D> _specular_fbo_tex;
    std::unique_ptr<Texture_cubemap> _point_shadow_fbo_tex;
    std::unique_ptr<Texture_2D> _point_shadow_fbo_depth_tex; // TODO: should be renderbuffer
    std::unique_ptr<Texture_2D> _spot_dir_shadow_fbo_tex;

    // simple quad used for rendering effects
    Quad _quad;

    Font_sys _font;
    Static_text _s_text;

    // entity tables
    std::vector<Entity> _ents; // TODO: list might be better
    // std::vector<Model> _model_components;
    // std::vector<Input> _input_components;
    // std::vector<Physics> _physics_components;

    Entity * _cam;
    Entity * _player;
};

#endif // WORLD_HPP
