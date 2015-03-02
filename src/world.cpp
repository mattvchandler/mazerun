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

// TODO: we've got a lot of state saving & restoring already. after code works, do performance sweep and clean these up when possible

// TODO: why uber-lag when resized? I think it's too much geometry... (but it shouldn't be)

#include "world.hpp"

#include <chrono>
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

#include "gl_helpers.hpp" // TODO: move w/ walls decl
#include "player.hpp"
#include "testmdl.hpp"

thread_local std::mt19937 prng;
thread_local std::random_device rng;

const unsigned int max_point_lights = 10;

Glew_init::Glew_init()
{
    if(Glew_init::_initialized)
    {
        // TODO: throw
    }
    Glew_init::_initialized = true;
    if(glewInit() != GLEW_OK)
    {
        std::cerr<<"Error loading glew"<<std::endl;
        // TODO: throw
    }
}

bool Glew_init::_initialized = false;

World::World():
    _win(sf::VideoMode(800, 600), "mazerun", sf::Style::Default, sf::ContextSettings(24, 8, 8, 3, 0)),
    _running(true), _focused(true), _do_resize(false),
    _sunlight(glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, glm::normalize(glm::vec3(-1.0f))),
    _walls(32, 32), _floor(32, 32),
    _ent_shader({std::make_pair("shaders/ents.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/ents.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0), std::make_pair("vert_tex_coords", 1),
        std::make_pair("vert_normals", 2), std::make_pair("vert_tangents", 3)}),
    _ents({create_player(), create_testmdl(), create_testlight()}),
    _cam(_ents[0])
{
    // TODO: loading screen

    _win.setKeyRepeatEnabled(false);
    // _win.setFramerateLimit(60);
    // TODO _win.setIcon

    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.0f, 1.0f);
    glLineWidth(5.0f);

    // TODO: enable backface culling
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendColor(1.0f, 1.0f, 1.0f, 0.1f);
    glEnable(GL_BLEND);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    resize();

    _ent_shader.use();
    _ent_shader.add_uniform("model_view_proj");
    _ent_shader.add_uniform("model_view");
    _ent_shader.add_uniform("normal_transform");
    _ent_shader.add_uniform("material.specular_color");
    _ent_shader.add_uniform("material.diffuse_map");
    _ent_shader.add_uniform("material.normal_map");
    _ent_shader.add_uniform("material.emissive_map");
    _ent_shader.add_uniform("material.shininess");
    _ent_shader.add_uniform("ambient_color");
    _ent_shader.add_uniform("num_point_lights");

    for(size_t i = 0; i < max_point_lights; ++i)
    {
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].base.color");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].base.strength");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].pos_eye");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].const_atten");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].linear_atten");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].quad_atten");
    }

    _ent_shader.add_uniform("dir_light.base.color");
    _ent_shader.add_uniform("dir_light.base.strength");
    _ent_shader.add_uniform("dir_light.dir");
    _ent_shader.add_uniform("dir_light.half_vec");

    _ent_shader.add_uniform("cam_light_forward");

    // set up static uniform vals
    // TODO: sunlight owned by skybox?
    // TODO: replace uniform bracket op w/ at so exceptions are thrown
    glm::vec3 ambient_color(0.2f, 0.2f, 0.2f); // TODO: get from skybox?
    glUniform3fv(_ent_shader.uniforms["ambient_color"], 1, &ambient_color[0]);
    glUniform3fv(_ent_shader.uniforms["dir_light.base.color"], 1, &_sunlight.color[0]); // TODO: Also from skybox?
    glUniform1f(_ent_shader.uniforms["dir_light.base.strength"], _sunlight.strength); // TODO: Also from skybox?
    glUniform1i(_ent_shader.uniforms["material.diffuse_map"], 0);
    glUniform1i(_ent_shader.uniforms["material.normal_map"], 1);
    glUniform1i(_ent_shader.uniforms["material.emissive_map"], 2);

    glUseProgram(0); // TODO get prev val
    check_error("World::World");
}

// TODO: picking. Should we always do a pick pass, or make a 'pick' method?
    // need: picking shader, target ent ptr
void World::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _ent_shader.use();

    // TODO: deferred lighting
    glm::vec3 cam_light_forward(0.0f, 0.0f, 1.0f); // in eye space
    glm::vec3 sunlight_dir = glm::transpose(glm::inverse(glm::mat3(_cam.view_mat()))) *
        glm::normalize(-_sunlight.dir);
    glUniform3fv(_ent_shader.uniforms["dir_light.dir"], 1, &sunlight_dir[0]);

    glm::vec3 sunlight_half_vec = glm::normalize(cam_light_forward + sunlight_dir);
    glUniform3fv(_ent_shader.uniforms["dir_light.half_vec"], 1, &sunlight_half_vec[0]);

    size_t point_light_i = 0;
    for(auto & ent: _ents)
    {
        std::shared_ptr<Light> light = ent.light();
        if(!light)
            continue;

        std::shared_ptr<Point_light> point_light = std::dynamic_pointer_cast<Point_light>(light);
        if(point_light)
        {
            glm::vec3 point_light_pos_eye = glm::vec3(_cam.view_mat() * ent.model_mat() *
                glm::vec4(point_light->pos, 1.0f));

            glUniform3fv(_ent_shader.uniforms["point_lights[" + std::to_string(point_light_i) + "].base.color"], 1, &point_light->color[0]);
            glUniform1f(_ent_shader.uniforms["point_lights[" + std::to_string(point_light_i) + "].base.strength"], point_light->strength);
            glUniform3fv(_ent_shader.uniforms["point_lights[" + std::to_string(point_light_i) + "].pos_eye"], 1, &point_light_pos_eye[0]);
            glUniform1f(_ent_shader.uniforms["point_lights[" + std::to_string(point_light_i) + "].const_atten"], point_light->const_atten);
            glUniform1f(_ent_shader.uniforms["point_lights[" + std::to_string(point_light_i) + "].linear_atten"], point_light->linear_atten);
            glUniform1f(_ent_shader.uniforms["point_lights[" + std::to_string(point_light_i) + "].quad_atten"], point_light->quad_atten);

            ++point_light_i;
        }
    }

    glUniform1i(_ent_shader.uniforms["num_point_lights"], point_light_i);

    check_error("World::draw - light setup");

    for(auto & ent: _ents)
    {
        auto model = ent.model();
        if(model)
        {
            glm::mat4 model_view = _cam.view_mat() * ent.model_mat();
            glm::mat4 model_view_proj = _proj * model_view;
            glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(model_view)));

            glUniformMatrix4fv(_ent_shader.uniforms["model_view_proj"], 1, GL_FALSE, &model_view_proj[0][0]);
            glUniformMatrix4fv(_ent_shader.uniforms["model_view"], 1, GL_FALSE, &model_view[0][0]);
            glUniformMatrix3fv(_ent_shader.uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

            glUniform3fv(_ent_shader.uniforms["material.specular_color"], 1, &model->get_material().specular_color[0]);
            glUniform1f(_ent_shader.uniforms["material.shininess"], model->get_material().shininess);
            glActiveTexture(GL_TEXTURE0);
            model->get_material().diffuse_map->bind();
            glActiveTexture(GL_TEXTURE1);
            model->get_material().normal_map->bind();
            glActiveTexture(GL_TEXTURE2);
            model->get_material().emissive_map->bind();

            model->draw();
        }
    }

    glm::mat4 model_view = _cam.view_mat();
    glm::mat4 model_view_proj = _proj * model_view;
    glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(model_view)));

    // sunlight vars
    glUniformMatrix4fv(_ent_shader.uniforms["model_view_proj"], 1, GL_FALSE, &model_view_proj[0][0]);
    glUniformMatrix4fv(_ent_shader.uniforms["model_view"], 1, GL_FALSE, &model_view[0][0]);
    glUniformMatrix3fv(_ent_shader.uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

    glUniform3fv(_ent_shader.uniforms["material.specular_color"], 1, &_walls.get_material().specular_color[0]);
    glUniform1f(_ent_shader.uniforms["material.shininess"], _walls.get_material().shininess);
    glActiveTexture(GL_TEXTURE0);
    _walls.get_material().diffuse_map->bind();
    glActiveTexture(GL_TEXTURE1);
    _walls.get_material().normal_map->bind();
    glActiveTexture(GL_TEXTURE2);
    _walls.get_material().emissive_map->bind();
    _walls.draw();

    glUniform3fv(_ent_shader.uniforms["material.specular_color"], 1, &_floor.get_material().specular_color[0]);
    glUniform1f(_ent_shader.uniforms["material.shininess"], _floor.get_material().shininess);
    glActiveTexture(GL_TEXTURE0);
    _floor.get_material().diffuse_map->bind();
    glActiveTexture(GL_TEXTURE1);
    _floor.get_material().normal_map->bind();
    glActiveTexture(GL_TEXTURE2);
    _floor.get_material().emissive_map->bind();
    _floor.draw();

    _skybox.draw(_cam, _proj);

    _win.display();
    check_error("World::draw - end");
}

// TODO: sound sys
    // need: sound service locator
    // listener
    // sound source component
    // OpenAL directly or SFML Audio?
        // OpenAL:
            // Pros: doppler, HRTF / other effects
            // Cons: more involved, no easy streaming, no load from file
        // SFML audio
            // Pros: easy, no extra library dep
            // cons: no special FX
        // use SFML

// TODO: logging

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

        if(!_running)
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

        // TODO should we make more threads for input, physics, messages, etc?
        // TODO: AI
        draw();
        _lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
        // TODO: framerate display
    }
}
