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
#include <iostream>
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

#include "entities/player.hpp"
#include "entities/testmdl.hpp"
#include "opengl/gl_helpers.hpp"
#include "util/logger.hpp"

thread_local std::mt19937 prng;
thread_local std::random_device rng;

extern std::atomic_bool interrupted; // defined in main.cpp

const unsigned int max_point_lights = 10; // TODO: get from config?
const unsigned int max_spot_lights = 10;

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
}

bool Glew_init::_initialized = false;

World::World():
    _win(sf::VideoMode(800, 600), "mazerun", sf::Style::Default, sf::ContextSettings(24, 8, 8)),
    _running(true), _focused(true), _do_resize(false),
    _sunlight(true, glm::vec3(1.0f, 1.0f, 1.0f), true, glm::normalize(glm::vec3(-1.0f))),
    _ent_prepass({std::make_pair("shaders/prepass.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/prepass.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0), std::make_pair("vert_tex_coords", 1),
        std::make_pair("vert_normals", 2), std::make_pair("vert_tangents", 3)},
        {std::make_pair("pos", 0), std::make_pair("g_shininess", 1), std::make_pair("g_norm", 2)}),
    // _ent_shader({std::make_pair("shaders/ents.vert", GL_VERTEX_SHADER),
    //     std::make_pair("shaders/ents.frag", GL_FRAGMENT_SHADER),
    //     std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
    //     {std::make_pair("vert_pos", 0), std::make_pair("vert_tex_coords", 1),
    //     std::make_pair("vert_normals", 2), std::make_pair("vert_tangents", 3)}),
    // _shadow_map_shader({std::make_pair("shaders/shadow.vert", GL_VERTEX_SHADER),
    //     std::make_pair("shaders/shadow.frag", GL_FRAGMENT_SHADER)},
    //     {std::make_pair("vert_pos", 0)}),
    _g_fbo(1024, 1024)
{
    Logger_locator::get()(Logger::TRACE, "World init starting...");
    // TODO: loading screen
    _ents.emplace_back(create_player());
    _ents.emplace_back(create_testmdl());
    _ents.emplace_back(create_testlight());
    _ents.emplace_back(create_testmonkey());
    _ents.emplace_back(create_walls(32, 32));
    _ents.emplace_back(create_floor(32, 32));

    _cam = _player = &_ents[0];

    _win.setKeyRepeatEnabled(false);
    // _win.setFramerateLimit(60);
    // TODO _win.setIcon

    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.0f, 1.0f);
    glLineWidth(5.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendColor(1.0f, 1.0f, 1.0f, 0.1f);
    glEnable(GL_BLEND);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    resize();

    _ent_prepass.use();
    _ent_prepass.add_uniform("model_view_proj");
    _ent_prepass.add_uniform("model_view");
    _ent_prepass.add_uniform("normal_transform");
    _ent_prepass.add_uniform("material.shininess");
    _ent_prepass.add_uniform("material.shininess_map");
    _ent_prepass.add_uniform("material.normal_map");
    glUniform1i(_ent_prepass.get_uniform("material.shininess_map"), 3);
    glUniform1i(_ent_prepass.get_uniform("material.normal_map"), 6);

    /*
    _ent_shader.use();
    _ent_shader.add_uniform("model_view_proj");
    _ent_shader.add_uniform("model_view");
    _ent_shader.add_uniform("model");
    _ent_shader.add_uniform("normal_transform");

    _ent_shader.add_uniform("material.ambient_color");
    _ent_shader.add_uniform("material.diffuse_color");
    _ent_shader.add_uniform("material.specular_color");
    _ent_shader.add_uniform("material.shininess");
    _ent_shader.add_uniform("material.emissive_color");
    // _ent_shader.add_uniform("material.reflectivity");

    _ent_shader.add_uniform("material.ambient_map");
    _ent_shader.add_uniform("material.diffuse_map");
    _ent_shader.add_uniform("material.specular_map");
    _ent_shader.add_uniform("material.shininess_map");
    _ent_shader.add_uniform("material.emissive_map");
    // _ent_shader.add_uniform("material.reflectivity_map");
    _ent_shader.add_uniform("material.normal_map");

    _ent_shader.add_uniform("ambient_light_color");
    _ent_shader.add_uniform("num_point_lights");
    _ent_shader.add_uniform("num_spot_lights");

    for(std::size_t i = 0; i < max_point_lights; ++i)
    {
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].base.enabled");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].base.color");
        // _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].base.casts_shadow");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].pos_eye");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].const_atten");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].linear_atten");
        _ent_shader.add_uniform("point_lights[" + std::to_string(i) + "].quad_atten");

        // we won't ever send the GPU disabled point lights
        glUniform1i(_ent_shader.get_uniform("point_lights[" + std::to_string(i) + "].base.enabled"), true);
    }

    for(std::size_t i = 0; i < max_spot_lights; ++i)
    {
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].base.enabled");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].base.color");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].base.casts_shadow");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].pos_eye");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].dir_eye");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].cos_cutoff");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].exponent");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].const_atten");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].linear_atten");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].quad_atten");
        _ent_shader.add_uniform("spot_lights[" + std::to_string(i) + "].shadow_mat");

        // we won't ever send the GPU disabled spot lights
        glUniform1i(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].base.enabled"), true);
    }
    // TODO: work around GLSL texture limits
    _ent_shader.add_uniform("shadow_map0");
    _ent_shader.add_uniform("shadow_map1");
    glUniform1i(_ent_shader.get_uniform("shadow_map0"), 7); // TODO: replace magic number
    glUniform1i(_ent_shader.get_uniform("shadow_map1"), 8); // TODO: replace magic number

    _ent_shader.add_uniform("dir_light.base.enabled");
    _ent_shader.add_uniform("dir_light.base.color");
    // _ent_shader.add_uniform("dir_light.base.casts_shadow");
    _ent_shader.add_uniform("dir_light.dir");
    _ent_shader.add_uniform("dir_light.half_vec");

    _ent_shader.add_uniform("cam_light_forward");

    // set up static uniform vals
    glUniform1i(_ent_shader.get_uniform("material.ambient_map"), 0);
    glUniform1i(_ent_shader.get_uniform("material.diffuse_map"), 1);
    glUniform1i(_ent_shader.get_uniform("material.specular_map"), 2);
    glUniform1i(_ent_shader.get_uniform("material.shininess_map"), 3);
    glUniform1i(_ent_shader.get_uniform("material.emissive_map"), 4);
    // glUniform1i(_ent_shader.get_uniform("material.reflectivity_map"), 5);
    glUniform1i(_ent_shader.get_uniform("material.normal_map"), 6);

    _shadow_map_shader.use();
    _shadow_map_shader.add_uniform("model_view_proj");
    */

    glUseProgram(0); // TODO get prev val?
    check_error("World::World");

    Message_locator::get().add_callback_empty("sun_toggle", [this](){ _sunlight.enabled = !_sunlight.enabled; });
}

// TODO: picking. Should we always do a pick pass, or make a 'pick' method?
    // need: picking shader, target ent ptr
void World::draw()
{
    glClearDepth(1.0f);
    /*
    // shadow map pass
    _shadow_map_shader.use();
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    std::size_t num_point_lights = 0;
    std::size_t num_spot_lights = 0;

    glm::mat4 spot_shadow_mat[max_spot_lights];
    Entity * point_lights[max_point_lights];
    Entity * spot_lights[max_spot_lights];

    const glm::mat4 scale_bias_mat(
        glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.5f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
        glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    auto set_no_material = [this](const Material & mat){};

    for(auto & ent: _ents)
    {
        Light * light = ent.light();
        if(!light || !light->enabled)
            continue;

        Point_light * point_light = dynamic_cast<Point_light *>(light);
        if(point_light && num_point_lights < max_point_lights)
        {
            point_lights[num_point_lights++] = &ent;
        }

        Spot_light * spot_light = dynamic_cast<Spot_light *>(light);
        if(spot_light && num_spot_lights < max_spot_lights)
        {
            spot_lights[num_spot_lights++] = &ent;

            if(!spot_light->casts_shadow || num_spot_lights > 2) // TODO: work around this limit
                continue;

            spot_light->shadow_fbo->bind_fbo();
            glViewport(0, 0, spot_light->shadow_fbo->get_width(), spot_light->shadow_fbo->get_height());
            glClear(GL_DEPTH_BUFFER_BIT);

            glm::mat4 view_proj = spot_light->shadow_proj_mat() * spot_light->shadow_view_mat() * ent.view_mat();
            spot_shadow_mat[num_spot_lights - 1] = scale_bias_mat * view_proj;

            for(auto & ent_2: _ents)
            {
                auto model = ent_2.model();
                if(!model || !model->casts_shadow)
                    continue;

                glm::mat4 model_view_proj = view_proj * ent_2.model_mat();
                glUniformMatrix4fv(_shadow_map_shader.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);

                model->draw(set_no_material);
            }
        }

        if(num_point_lights >= max_point_lights && num_spot_lights >= max_spot_lights)
            break;
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    _ent_shader.use();

    check_error("World::draw - shadow pass");

    glViewport(0, 0, _win.getSize().x, _win.getSize().y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: point & dir shadows
    // TODO: forward+ rendering - see http://www.adriancourreges.com/blog/2015/03/10/deus-ex-human-revolution-graphics-study/
    // TODO: sunlight owned by skybox?
    glm::vec3 ambient_light_color(0.1f, 0.1f, 0.1f); // TODO: get from skybox?
    glm::vec3 cam_light_forward(0.0f, 0.0f, 1.0f); // in eye space
    glUniform3fv(_ent_shader.get_uniform("ambient_light_color"), 1, &ambient_light_color[0]);
    glUniform1i(_ent_shader.get_uniform("dir_light.base.enabled"), _sunlight.enabled); // TODO: Also from skybox?
    if(_sunlight.enabled)
    {
        glm::vec3 sunlight_dir = glm::normalize(glm::transpose(glm::inverse(glm::mat3(_cam->view_mat()))) *
            glm::normalize(-_sunlight.dir));
        glm::vec3 sunlight_half_vec = glm::normalize(cam_light_forward + sunlight_dir);

        glUniform3fv(_ent_shader.get_uniform("dir_light.base.color"), 1, &_sunlight.color[0]); // TODO: Also from skybox?
        glUniform3fv(_ent_shader.get_uniform("dir_light.dir"), 1, &sunlight_dir[0]);
        glUniform3fv(_ent_shader.get_uniform("dir_light.half_vec"), 1, &sunlight_half_vec[0]);
    }

    for(std::size_t i = 0; i < num_point_lights; ++i)
    {
        Entity & ent = *point_lights[i];
        Point_light & point_light = *dynamic_cast<Point_light *>(ent.light());

        glm::mat4 model_view = _cam->view_mat() * ent.model_mat();
        glm::vec3 point_light_pos_eye = glm::vec3(model_view * glm::vec4(point_light.pos, 1.0f));

        glUniform3fv(_ent_shader.get_uniform("point_lights[" + std::to_string(i) + "].base.color"), 1, &point_light.color[0]);
        glUniform3fv(_ent_shader.get_uniform("point_lights[" + std::to_string(i) + "].pos_eye"), 1, &point_light_pos_eye[0]);
        glUniform1f(_ent_shader.get_uniform("point_lights[" + std::to_string(i) + "].const_atten"), point_light.const_atten);
        glUniform1f(_ent_shader.get_uniform("point_lights[" + std::to_string(i) + "].linear_atten"), point_light.linear_atten);
        glUniform1f(_ent_shader.get_uniform("point_lights[" + std::to_string(i) + "].quad_atten"), point_light.quad_atten);
    }

    glUniform1i(_ent_shader.get_uniform("num_point_lights"), num_point_lights);

    for(std::size_t i = 0; i < num_spot_lights; ++i)
    {
        Entity & ent = *spot_lights[i];
        Spot_light & spot_light = *dynamic_cast<Spot_light *>(ent.light());

        glm::mat4 model_view = _cam->view_mat() * ent.model_mat();
        glm::vec3 spot_light_pos_eye = glm::vec3(model_view * glm::vec4(spot_light.pos, 1.0f));

        glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(model_view)));
        glm::vec3 spot_light_dir_eye = glm::normalize(normal_transform * spot_light.dir);

        glUniform3fv(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].base.color"), 1, &spot_light.color[0]);
        glUniform1i(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].base.casts_shadow"), spot_light.casts_shadow);
        glUniform3fv(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].pos_eye"), 1, &spot_light_pos_eye[0]);
        glUniform3fv(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].dir_eye"), 1, &spot_light_dir_eye[0]);
        glUniform1f(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].cos_cutoff"), spot_light.cos_cutoff);
        glUniform1f(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].exponent"), spot_light.exponent);
        glUniform1f(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].const_atten"), spot_light.const_atten);
        glUniform1f(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].linear_atten"), spot_light.linear_atten);
        glUniform1f(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].quad_atten"), spot_light.quad_atten);

        if(spot_light.casts_shadow && i < 2) // TODO: work around
        {
            glUniformMatrix4fv(_ent_shader.get_uniform("spot_lights[" + std::to_string(i) + "].shadow_mat"), 1, GL_FALSE, &spot_shadow_mat[i][0][0]);

            glActiveTexture(GL_TEXTURE0 + i + 7); // TODO: replace GL_TEXTURE7
            spot_light.shadow_fbo->bind_tex();
        }
    }

    glUniform1i(_ent_shader.get_uniform("num_spot_lights"), num_spot_lights);

    check_error("World::draw - light setup");

    */
    auto set_material = [this](const Material & mat)
    {
        // glUniform3fv(_ent_prepass.get_uniform("material.ambient_color"), 1, &mat.ambient_color[0]);
        // glUniform3fv(_ent_prepass.get_uniform("material.diffuse_color"), 1, &mat.diffuse_color[0]);
        // glUniform3fv(_ent_prepass.get_uniform("material.specular_color"), 1, &mat.specular_color[0]);
        glUniform1f(_ent_prepass.get_uniform("material.shininess"), mat.shininess);
        // glUniform3fv(_ent_prepass.get_uniform("material.emissive_color"), 1, &mat.emissive_color[0]);
        // glUniform1f(_ent_prepass.get_uniform("material.reflectivity"), mat.reflectivity);

        // glActiveTexture(GL_TEXTURE0);
        // mat.ambient_map->bind();
        // glActiveTexture(GL_TEXTURE1);
        // mat.diffuse_map->bind();
        // glActiveTexture(GL_TEXTURE2);
        // mat.specular_map->bind();
        glActiveTexture(GL_TEXTURE3);
        mat.shininess_map->bind();
        // glActiveTexture(GL_TEXTURE4);
        // mat.emissive_map->bind();
        // glActiveTexture(GL_TEXTURE5);
        // mat.reflectivity_map->bind();
        glActiveTexture(GL_TEXTURE6);
        mat.normal_map->bind();
    };

    _g_fbo.bind_fbo();
    _ent_prepass.use();
    glViewport(0, 0, _g_fbo.get_width(), _g_fbo.get_height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(auto & ent: _ents)
    {
        auto model = ent.model();
        if(model)
        {
            glm::mat4 model_view = _cam->view_mat() * ent.model_mat();
            glm::mat4 model_view_proj = _proj * model_view;
            glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(model_view)));

            glUniformMatrix4fv(_ent_prepass.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);
            glUniformMatrix4fv(_ent_prepass.get_uniform("model_view"), 1, GL_FALSE, &model_view[0][0]);
            glUniformMatrix3fv(_ent_prepass.get_uniform("normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

            model->draw(set_material);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _win.getSize().x, _win.getSize().y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _skybox.draw(*_cam, _proj);

    _win.display();
    check_error("World::draw - end");
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
        // glm::vec3 cam_up = _cam->up();
        sf::Listener::setPosition(cam_pos.x, cam_pos.y, cam_pos.z);
        sf::Listener::setDirection(cam_forward.x, cam_forward.y, cam_forward.z);
        // sf::Listener::setUpVector(cam_up.x, cam_up.y, cam_up.z); // TODO: available in SFML 2.2

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
        // TODO: framerate display
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
