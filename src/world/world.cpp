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

// const unsigned int max_point_lights = 10; // TODO: get from config?
// const unsigned int max_spot_lights = 10;

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
    _point_light_prog({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/point_light.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _point_light_shadow_prog({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/point_light_shadow.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _spot_light_prog({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/spot_light.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _spot_light_shadow_prog({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/spot_light_shadow.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _dir_light_prog({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/dir_light.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _dir_light_shadow_prog({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/dir_light_shadow.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _point_shadow_prog({std::make_pair("shaders/point_shadow.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/point_shadow.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _spot_dir_shadow_prog({std::make_pair("shaders/shadow.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/shadow.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _set_depth_prog({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/set_depth.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _ent_shader({std::make_pair("shaders/ents.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/ents.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0), std::make_pair("vert_tex_coords", 1)}),
    // TODO: what should FBO sizes be?
    _fullscreen_tex({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER), // TODO: not needed?
        std::make_pair("shaders/just-texture.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _g_fbo_pos_tex(FBO::create_tex(800, 600)), // TODO: resize
    _g_fbo_shininess_tex(FBO::create_tex(800, 600)),
    _g_fbo_normal_tex(FBO::create_tex(800, 600)),
    _g_fbo_depth_tex(FBO::create_depth_tex(800, 600)),
    _diffuse_fbo_tex(FBO::create_tex(800, 600)),
    _specular_fbo_tex(FBO::create_tex(800, 600)),
    _point_shadow_fbo_tex(FBO::create_shadow_cube_tex(512, 512)),
    _point_shadow_fbo_depth_tex(FBO::create_depth_tex(512, 512)),
    _spot_dir_shadow_fbo_tex(FBO::create_shadow_tex(512, 512)),
    _font("Symbola", 18),
    _s_text(_font, u8"ASDF … more pages now‽: Á🐙💩☹☢☣☠\u0301",
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec2(10.0f, 30.0f))
{
    // TODO: standardize naming
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

    glDepthRangef(0.0f, 1.0f);
    glLineWidth(5.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    resize();

    const glm::vec3 cam_light_forward(0.0f, 0.0f, 1.0f); // in eye space

    // Uniform setup
    _ent_prepass.use();
    _ent_prepass.add_uniform("model_view_proj");
    _ent_prepass.add_uniform("model_view");
    _ent_prepass.add_uniform("normal_transform");
    _ent_prepass.add_uniform("material.shininess");
    _ent_prepass.add_uniform("material.shininess_map");
    _ent_prepass.add_uniform("material.normal_map");
    glUniform1i(_ent_prepass.get_uniform("material.shininess_map"), 0);
    glUniform1i(_ent_prepass.get_uniform("material.normal_map"), 1);

    _point_light_prog.use();
    _point_light_prog.add_uniform("point_light.base.color");
    _point_light_prog.add_uniform("point_light.pos_eye");
    _point_light_prog.add_uniform("point_light.const_atten");
    _point_light_prog.add_uniform("point_light.linear_atten");
    _point_light_prog.add_uniform("point_light.quad_atten");
    _point_light_prog.add_uniform("pos_map");
    _point_light_prog.add_uniform("shininess_map");
    _point_light_prog.add_uniform("normal_map");
    _point_light_prog.add_uniform("viewport_size");
    _point_light_prog.add_uniform("cam_light_forward");
    glUniform1i(_point_light_prog.get_uniform("pos_map"), 0);
    glUniform1i(_point_light_prog.get_uniform("shininess_map"), 1);
    glUniform1i(_point_light_prog.get_uniform("normal_map"), 2);
    glUniform3fv(_point_light_prog.get_uniform("cam_light_forward"), 1, &cam_light_forward[0]);

    _point_light_shadow_prog.use();
    _point_light_shadow_prog.add_uniform("point_light.base.color");
    _point_light_shadow_prog.add_uniform("point_light.pos_eye");
    _point_light_shadow_prog.add_uniform("point_light.const_atten");
    _point_light_shadow_prog.add_uniform("point_light.linear_atten");
    _point_light_shadow_prog.add_uniform("point_light.quad_atten");
    _point_light_shadow_prog.add_uniform("pos_map");
    _point_light_shadow_prog.add_uniform("shininess_map");
    _point_light_shadow_prog.add_uniform("normal_map");
    _point_light_shadow_prog.add_uniform("viewport_size");
    _point_light_shadow_prog.add_uniform("cam_light_forward");
    _point_light_shadow_prog.add_uniform("shadow_map");
    _point_light_shadow_prog.add_uniform("light_world_pos");
    _point_light_shadow_prog.add_uniform("inv_cam_view");
    glUniform1i(_point_light_shadow_prog.get_uniform("pos_map"), 0);
    glUniform1i(_point_light_shadow_prog.get_uniform("shininess_map"), 1);
    glUniform1i(_point_light_shadow_prog.get_uniform("normal_map"), 2);
    glUniform1i(_point_light_shadow_prog.get_uniform("shadow_map"), 3);
    glUniform3fv(_point_light_shadow_prog.get_uniform("cam_light_forward"), 1, &cam_light_forward[0]);

    _spot_light_prog.use();
    _spot_light_prog.add_uniform("spot_light.base.color");
    _spot_light_prog.add_uniform("spot_light.pos_eye");
    _spot_light_prog.add_uniform("spot_light.dir_eye");
    _spot_light_prog.add_uniform("spot_light.cos_cutoff");
    _spot_light_prog.add_uniform("spot_light.exponent");
    _spot_light_prog.add_uniform("spot_light.const_atten");
    _spot_light_prog.add_uniform("spot_light.linear_atten");
    _spot_light_prog.add_uniform("spot_light.quad_atten");
    _spot_light_prog.add_uniform("pos_map");
    _spot_light_prog.add_uniform("shininess_map");
    _spot_light_prog.add_uniform("normal_map");
    _spot_light_prog.add_uniform("viewport_size");
    _spot_light_prog.add_uniform("cam_light_forward");
    glUniform1i(_spot_light_prog.get_uniform("pos_map"), 0);
    glUniform1i(_spot_light_prog.get_uniform("shininess_map"), 1);
    glUniform1i(_spot_light_prog.get_uniform("normal_map"), 2);
    glUniform3fv(_spot_light_prog.get_uniform("cam_light_forward"), 1, &cam_light_forward[0]);

    _spot_light_shadow_prog.use();
    _spot_light_shadow_prog.add_uniform("spot_light.base.color");
    _spot_light_shadow_prog.add_uniform("spot_light.pos_eye");
    _spot_light_shadow_prog.add_uniform("spot_light.dir_eye");
    _spot_light_shadow_prog.add_uniform("spot_light.cos_cutoff");
    _spot_light_shadow_prog.add_uniform("spot_light.exponent");
    _spot_light_shadow_prog.add_uniform("spot_light.const_atten");
    _spot_light_shadow_prog.add_uniform("spot_light.linear_atten");
    _spot_light_shadow_prog.add_uniform("spot_light.quad_atten");
    _spot_light_shadow_prog.add_uniform("pos_map");
    _spot_light_shadow_prog.add_uniform("shininess_map");
    _spot_light_shadow_prog.add_uniform("normal_map");
    _spot_light_shadow_prog.add_uniform("viewport_size");
    _spot_light_shadow_prog.add_uniform("cam_light_forward");
    _spot_light_shadow_prog.add_uniform("shadow_mat");
    _spot_light_shadow_prog.add_uniform("shadow_map");
    glUniform1i(_spot_light_shadow_prog.get_uniform("pos_map"), 0);
    glUniform1i(_spot_light_shadow_prog.get_uniform("shininess_map"), 1);
    glUniform1i(_spot_light_shadow_prog.get_uniform("normal_map"), 2);
    glUniform1i(_spot_light_shadow_prog.get_uniform("shadow_map"), 3);
    glUniform3fv(_spot_light_shadow_prog.get_uniform("cam_light_forward"), 1, &cam_light_forward[0]);

    _dir_light_prog.use();
    _dir_light_prog.add_uniform("dir_light.base.color");
    _dir_light_prog.add_uniform("dir_light.dir");
    _dir_light_prog.add_uniform("dir_light.half_vec");
    _dir_light_prog.add_uniform("shininess_map");
    _dir_light_prog.add_uniform("normal_map");
    _dir_light_prog.add_uniform("viewport_size");
    glUniform1i(_dir_light_prog.get_uniform("shininess_map"), 1);
    glUniform1i(_dir_light_prog.get_uniform("normal_map"), 2);

    _dir_light_shadow_prog.use();
    _dir_light_shadow_prog.add_uniform("dir_light.base.color");
    _dir_light_shadow_prog.add_uniform("dir_light.dir");
    _dir_light_shadow_prog.add_uniform("dir_light.half_vec");
    _dir_light_shadow_prog.add_uniform("pos_map");
    _dir_light_shadow_prog.add_uniform("shininess_map");
    _dir_light_shadow_prog.add_uniform("normal_map");
    _dir_light_shadow_prog.add_uniform("viewport_size");
    _dir_light_shadow_prog.add_uniform("shadow_map");
    _dir_light_shadow_prog.add_uniform("shadow_mat");
    glUniform1i(_dir_light_shadow_prog.get_uniform("pos_map"), 0);
    glUniform1i(_dir_light_shadow_prog.get_uniform("shininess_map"), 1);
    glUniform1i(_dir_light_shadow_prog.get_uniform("normal_map"), 2);
    glUniform1i(_dir_light_shadow_prog.get_uniform("shadow_map"), 3);

    _point_shadow_prog.use();
    _point_shadow_prog.add_uniform("model_view_proj");
    _point_shadow_prog.add_uniform("model");
    _point_shadow_prog.add_uniform("light_world_pos");

    _spot_dir_shadow_prog.use();
    _spot_dir_shadow_prog.add_uniform("model_view_proj");

    _set_depth_prog.use();
    _set_depth_prog.add_uniform("viewport_size");
    _set_depth_prog.add_uniform("tex");
    glUniform1i(_set_depth_prog.get_uniform("tex"), 0);

    _ent_shader.use();
    _ent_shader.add_uniform("model_view_proj");
    _ent_shader.add_uniform("material.ambient_color");
    _ent_shader.add_uniform("material.diffuse_color");
    _ent_shader.add_uniform("material.specular_color");
    _ent_shader.add_uniform("material.emissive_color");
    // _ent_shader.add_uniform("material.reflectivity");
    _ent_shader.add_uniform("material.ambient_map");
    _ent_shader.add_uniform("material.diffuse_map");
    _ent_shader.add_uniform("material.specular_map");
    _ent_shader.add_uniform("material.emissive_map");
    // _ent_shader.add_uniform("material.reflectivity_map");
    _ent_shader.add_uniform("ambient_light_color");
    _ent_shader.add_uniform("diffuse_fbo_tex");
    _ent_shader.add_uniform("specular_fbo_tex");
    _ent_shader.add_uniform("viewport_size");

    // set up static uniform vals
    glUniform1i(_ent_shader.get_uniform("material.ambient_map"), 0);
    glUniform1i(_ent_shader.get_uniform("material.diffuse_map"), 1);
    glUniform1i(_ent_shader.get_uniform("material.specular_map"), 2);
    glUniform1i(_ent_shader.get_uniform("material.emissive_map"), 3);
    // glUniform1i(_ent_shader.get_uniform("material.reflectivity_map"), 4);
    glUniform1i(_ent_shader.get_uniform("diffuse_fbo_tex"), 5);
    glUniform1i(_ent_shader.get_uniform("specular_fbo_tex"), 6);

    // TODO: remove
    _fullscreen_tex.use();
    _fullscreen_tex.add_uniform("viewport_size");
    _fullscreen_tex.add_uniform("tex");
    glUniform1i(_fullscreen_tex.get_uniform("tex"), 0);

    glUseProgram(0); // TODO get prev val?

    // setup FBOs
    _g_fbo.bind();

    // TODO: put shininess in the alpha channel for depth
    int i = 0;
    for(auto tex_id: {_g_fbo_pos_tex->get_id(), _g_fbo_shininess_tex->get_id(), _g_fbo_normal_tex->get_id()})
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (i++), GL_TEXTURE_2D, tex_id, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _g_fbo_depth_tex->get_id(), 0);

    const GLenum buffs[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, buffs);

    _g_fbo.verify();

    _lighting_fbo.bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _diffuse_fbo_tex->get_id(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _specular_fbo_tex->get_id(), 0);
    glDrawBuffers(2, buffs);

    _lighting_fbo.verify();

    _point_shadow_fbo.bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, _point_shadow_fbo_tex->get_id(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _point_shadow_fbo_depth_tex->get_id(), 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    _point_shadow_fbo.verify();

    _spot_dir_shadow_fbo.bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _spot_dir_shadow_fbo_tex->get_id(), 0);
    glDrawBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Message_locator::get().add_callback_empty("sun_toggle", [this](){ _sunlight.enabled = !_sunlight.enabled; });

    check_error("World::World");
}

// TODO: picking. Should we always do a pick pass, or make a 'pick' method?
    // need: picking shader, target ent ptr
// TODO: weird lighting artifacts
// TODO: FPS display
void World::draw()
{
    // TODO: no lighting until camera moved, and when pointed down
    const glm::vec3 cam_light_forward(0.0f, 0.0f, 1.0f); // in eye space

    std::vector<Entity *> point_lights;
    std::vector<Entity *> spot_lights;
    std::vector<Entity *> models;
    point_lights.reserve(_ents.size());
    spot_lights.reserve(_ents.size());
    models.reserve(_ents.size());

    auto set_prepass_material = [this](const Material & mat)
    {
        glUniform1f(_ent_prepass.get_uniform("material.shininess"), mat.shininess);
        glActiveTexture(GL_TEXTURE0);
        mat.shininess_map->bind();
        glActiveTexture(GL_TEXTURE1);
        mat.normal_map->bind();
    };

    glm::vec2 viewport_size = {(float)800, (float)600}; // TODO: how to get fbo size?

    _g_fbo.bind();
    glViewport(0, 0, 800, 600); // TODO: how to get fbo size?

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);

    _ent_prepass.use();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(auto & ent: _ents)
    {
        auto model = ent.model();
        if(model)
        {
            models.push_back(&ent);
            glm::mat4 model_view = _cam->view_mat() * ent.model_mat();
            glm::mat4 model_view_proj = _proj * model_view;
            glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(model_view)));

            glUniformMatrix4fv(_ent_prepass.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);
            glUniformMatrix4fv(_ent_prepass.get_uniform("model_view"), 1, GL_FALSE, &model_view[0][0]);
            glUniformMatrix3fv(_ent_prepass.get_uniform("normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

            model->draw(set_prepass_material);
            check_error("World::draw - prepass");
        }

        // collect lighting info
        auto light = ent.light();
        if(light && light->enabled)
        {
            Point_light * point_light = dynamic_cast<Point_light *>(light);
            if(point_light)
            {
                point_lights.push_back(&ent);
            }

            Spot_light * spot_light = dynamic_cast<Spot_light *>(light);
            if(spot_light)
            {
                spot_lights.push_back(&ent);
            }
        }
    }

    // Lighting pass
    const glm::mat4 scale_bias_mat(
        glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.5f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
        glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    glm::mat4 inv_cam_view = glm::inverse(_cam->view_mat());

    _lighting_fbo.bind();

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glActiveTexture(GL_TEXTURE0);
    _g_fbo_pos_tex->bind();
    glActiveTexture(GL_TEXTURE1);
    _g_fbo_shininess_tex->bind();
    glActiveTexture(GL_TEXTURE2);
    _g_fbo_normal_tex->bind();
    glActiveTexture(GL_TEXTURE3);
    _point_shadow_fbo_tex->bind();

    glClear(GL_COLOR_BUFFER_BIT);

    _point_light_shadow_prog.use();
    glUniform2fv(_point_light_shadow_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

    _point_light_prog.use();
    glUniform2fv(_point_light_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

    // common point lighting
    auto point_common = [this](const Shader_prog & point_prog, const Entity & ent, const Point_light & point_light)
    {
        glm::mat4 model_view = _cam->view_mat() * ent.model_mat();
        glm::vec3 point_light_pos_eye = glm::vec3(model_view * glm::vec4(point_light.pos, 1.0f));

        glUniform3fv(point_prog.get_uniform("point_light.base.color"), 1, &point_light.color[0]);
        glUniform3fv(point_prog.get_uniform("point_light.pos_eye"), 1, &point_light_pos_eye[0]);
        glUniform1f(point_prog.get_uniform("point_light.const_atten"), point_light.const_atten);
        glUniform1f(point_prog.get_uniform("point_light.linear_atten"), point_light.linear_atten);
        glUniform1f(point_prog.get_uniform("point_light.quad_atten"), point_light.quad_atten);

        _quad.draw(); // TODO: sphere or smaller quad instead?
    };

    bool use_shadow = false;

    for(auto & ent: point_lights)
    {
        Point_light * point_light = dynamic_cast<Point_light *>(ent->light());

        if(point_light->casts_shadow)
        {
            // TODO: blocky shadows
            // create shadow map
            _point_shadow_fbo.bind();
            glViewport(0, 0, 512, 512);
            _point_shadow_prog.use();
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glClearColor(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());

            glm::vec3 light_world_pos = glm::vec3(ent->model_mat() * glm::vec4(point_light->pos, 1.0f));

            for(const auto & dir: {
                GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                GL_TEXTURE_CUBE_MAP_NEGATIVE_Z})
            {
                glm::mat4 view_proj = point_light->shadow_proj_mat() * point_light->shadow_view_mat(dir) * glm::translate(glm::mat4(), -light_world_pos);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dir, _point_shadow_fbo_tex->get_id(), 0);
                _point_shadow_fbo.verify();

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                for(auto & ent_2: models)
                {
                    auto model = ent_2->model();
                    if(!model->casts_shadow)
                        continue;

                    glm::mat4 model_mat = ent_2->model_mat();
                    glm::mat4 model_view_proj = view_proj * model_mat;
                    glUniformMatrix4fv(_point_shadow_prog.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);
                    glUniformMatrix4fv(_point_shadow_prog.get_uniform("model"), 1, GL_FALSE, &model_mat[0][0]);
                    glUniform3fv(_point_shadow_prog.get_uniform("light_world_pos"), 1, &light_world_pos[0]);

                    model->draw([](const Material &){});
                    check_error("World::draw - spot light shadow map");
                }
            }

            _lighting_fbo.bind();
            glViewport(0, 0, 800, 600);
            _point_light_shadow_prog.use();
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_POLYGON_OFFSET_FILL);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

            glUniformMatrix4fv(_point_light_shadow_prog.get_uniform("inv_cam_view"), 1, GL_FALSE, &inv_cam_view[0][0]);
            glUniform3fv(_point_light_shadow_prog.get_uniform("light_world_pos"), 1, &light_world_pos[0]);

            point_common(_point_light_shadow_prog, *ent, *point_light);
            check_error("World::draw - point light shadow quad");

            use_shadow = true;
        }
        else
        {
            if(use_shadow)
            {
                use_shadow = false;
                _point_light_prog.use();
            }

            point_common(_point_light_prog, *ent, *point_light);
            check_error("World::draw - point light quad");
        }
    }

    glActiveTexture(GL_TEXTURE3);
    _spot_dir_shadow_fbo_tex->bind();

    glPolygonOffset(2.0f, 4.0f);

    _spot_light_shadow_prog.use();
    glUniform2fv(_spot_light_shadow_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

    _spot_light_prog.use();
    glUniform2fv(_spot_light_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

    // common spot lighting
    auto spot_common = [this](const Shader_prog & spot_prog, const Entity & ent, const Spot_light & spot_light)
    {
        glm::mat4 model_view = _cam->view_mat() * ent.model_mat();
        glm::vec3 spot_light_pos_eye = glm::vec3(model_view * glm::vec4(spot_light.pos, 1.0f));

        glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(model_view)));
        glm::vec3 spot_light_dir_eye = glm::normalize(normal_transform * spot_light.dir);

        glUniform3fv(spot_prog.get_uniform("spot_light.base.color"), 1, &spot_light.color[0]);
        glUniform3fv(spot_prog.get_uniform("spot_light.pos_eye"), 1, &spot_light_pos_eye[0]);
        glUniform3fv(spot_prog.get_uniform("spot_light.dir_eye"), 1, &spot_light_dir_eye[0]);
        glUniform1f(spot_prog.get_uniform("spot_light.cos_cutoff"), spot_light.cos_cutoff);
        glUniform1f(spot_prog.get_uniform("spot_light.exponent"), spot_light.exponent);
        glUniform1f(spot_prog.get_uniform("spot_light.const_atten"), spot_light.const_atten);
        glUniform1f(spot_prog.get_uniform("spot_light.linear_atten"), spot_light.linear_atten);
        glUniform1f(spot_prog.get_uniform("spot_light.quad_atten"), spot_light.quad_atten);

        _quad.draw(); // TODO: sphere or smaller quad instead?
    };

    use_shadow = false;

    for(auto & ent: spot_lights)
    {
        Spot_light * spot_light = dynamic_cast<Spot_light *>(ent->light());

        if(spot_light->casts_shadow)
        {
            // create shadow map
            _spot_dir_shadow_fbo.bind();
            glViewport(0, 0, 512, 512);
            _spot_dir_shadow_prog.use();
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glEnable(GL_POLYGON_OFFSET_FILL);

            glClear(GL_DEPTH_BUFFER_BIT);

            glm::mat4 view_proj = spot_light->shadow_proj_mat() * spot_light->shadow_view_mat() * ent->view_mat();
            glm::mat4 spot_shadow_mat = scale_bias_mat * view_proj * inv_cam_view;

            for(auto & ent_2: models)
            {
                auto model = ent_2->model();
                if(!model->casts_shadow)
                    continue;

                glm::mat4 model_view_proj = view_proj * ent_2->model_mat();
                glUniformMatrix4fv(_spot_dir_shadow_prog.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);

                model->draw([](const Material &){});
                check_error("World::draw - spot light shadow map");
            }

            _lighting_fbo.bind();
            glViewport(0, 0, 800, 600);
            _spot_light_shadow_prog.use();
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_POLYGON_OFFSET_FILL);

            glUniformMatrix4fv(_spot_light_shadow_prog.get_uniform("shadow_mat"), 1, GL_FALSE, &spot_shadow_mat[0][0]);
            spot_common(_spot_light_shadow_prog, *ent, *spot_light);
            check_error("World::draw - spot light shadow quad");

            use_shadow = true;
        }
        else
        {
            if(use_shadow)
            {
                use_shadow = false;
                _spot_light_prog.use();
            }

            spot_common(_spot_light_prog, *ent, *spot_light);
            check_error("World::draw - spot light quad");
        }
    }

    auto dir_common = [this](const Shader_prog & dir_prog, const glm::vec3 & cam_light_forward, const glm::vec2 & viewport_size)
    {
        glm::vec3 sunlight_dir = glm::normalize(glm::transpose(glm::inverse(glm::mat3(_cam->view_mat()))) *
            glm::normalize(-_sunlight.dir));
        glm::vec3 sunlight_half_vec = glm::normalize(cam_light_forward + sunlight_dir);

        glUniform3fv(dir_prog.get_uniform("dir_light.base.color"), 1, &_sunlight.color[0]); // TODO: Also from skybox?
        glUniform3fv(dir_prog.get_uniform("dir_light.dir"), 1, &sunlight_dir[0]);
        glUniform3fv(dir_prog.get_uniform("dir_light.half_vec"), 1, &sunlight_half_vec[0]);

        glUniform2fv(dir_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

        _quad.draw();
    };

    if(_sunlight.enabled)
    {
        if(_sunlight.casts_shadow)
        {
            // TODO: blocky shadows? cascaded shadow maps?
            //      http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
            //      https://gamedev.stackexchange.com/questions/68016/shadow-mapping-with-directional-light
            // create shadow map
            _spot_dir_shadow_fbo.bind();
            glViewport(0, 0, 512, 512);
            _spot_dir_shadow_prog.use();
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glEnable(GL_POLYGON_OFFSET_FILL);

            glClear(GL_DEPTH_BUFFER_BIT);

            glm::mat4 view_proj = _sunlight.shadow_proj_mat(45.0f, 45.0f, 45.0f) * _sunlight.shadow_view_mat();
            glm::mat4 dir_shadow_mat = scale_bias_mat * view_proj * glm::inverse(_cam->view_mat());

            for(auto & ent: models)
            {
                auto model = ent->model();
                if(!model->casts_shadow)
                    continue;

                glm::mat4 model_view_proj = view_proj * ent->model_mat();
                glUniformMatrix4fv(_spot_dir_shadow_prog.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);

                model->draw([](const Material &){});
                check_error("World::draw - dir light shadow map");
            }

            _lighting_fbo.bind();
            glViewport(0, 0, 800, 600);
            _dir_light_shadow_prog.use();
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_POLYGON_OFFSET_FILL);

            glUniformMatrix4fv(_dir_light_shadow_prog.get_uniform("shadow_mat"), 1, GL_FALSE, &dir_shadow_mat[0][0]);
            dir_common(_dir_light_shadow_prog, cam_light_forward, viewport_size);
            check_error("World::draw - dir light shadow quad");
        }
        else
        {
            _dir_light_prog.use();
            dir_common(_dir_light_prog, cam_light_forward, viewport_size);
            check_error("World::draw - dir light quad");
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _win.getSize().x, _win.getSize().y);
    viewport_size = {(float)_win.getSize().x, (float)_win.getSize().y};

    // set default framebuffer depth buffer from G buffer
    glDisable(GL_BLEND);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _set_depth_prog.use();
    glUniform2fv(_set_depth_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

    glActiveTexture(GL_TEXTURE0);
    _g_fbo_depth_tex->bind();
    _quad.draw();
    check_error("World::draw - default depthbuffer fill");

    // main drawing pass
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2.0f, 4.0f);

    _ent_shader.use();

    auto set_material = [this](const Material & mat)
    {
        glUniform3fv(_ent_shader.get_uniform("material.ambient_color"), 1, &mat.ambient_color[0]);
        glUniform3fv(_ent_shader.get_uniform("material.diffuse_color"), 1, &mat.diffuse_color[0]);
        glUniform3fv(_ent_shader.get_uniform("material.specular_color"), 1, &mat.specular_color[0]);
        glUniform3fv(_ent_shader.get_uniform("material.emissive_color"), 1, &mat.emissive_color[0]);
        // glUniform1f(_ent_shader.get_uniform("material.reflectivity"), mat.reflectivity);

        glActiveTexture(GL_TEXTURE0);
        mat.ambient_map->bind();
        glActiveTexture(GL_TEXTURE1);
        mat.diffuse_map->bind();
        glActiveTexture(GL_TEXTURE2);
        mat.specular_map->bind();
        glActiveTexture(GL_TEXTURE3);
        mat.emissive_map->bind();
        // glActiveTexture(GL_TEXTURE4);
        // mat.reflectivity_map->bind();
    };

    glActiveTexture(GL_TEXTURE5);
    _diffuse_fbo_tex->bind();
    glActiveTexture(GL_TEXTURE6);
    _specular_fbo_tex->bind();

    glUniform2fv(_ent_shader.get_uniform("viewport_size"), 1, &viewport_size[0]);

    for(auto & ent: models)
    {
        Model * model = ent->model();

        glm::mat4 model_view = _cam->view_mat() * ent->model_mat();
        glm::mat4 model_view_proj = _proj * model_view;

        glUniformMatrix4fv(_ent_shader.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);

        model->draw(set_material);
        check_error("World::draw - main pass");
    }
    glDisable(GL_POLYGON_OFFSET_FILL);

    _skybox.draw(*_cam, _proj);
    check_error("World::draw - skybox");

    // TODO: antialiasing

    _s_text.render_text(_font);

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
