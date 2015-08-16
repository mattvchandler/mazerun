// setup.cpp
// world setup

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

#include "world/world.hpp"

#include "entities/player.hpp"
#include "entities/testmdl.hpp"
#include "opengl/gl_helpers.hpp"
#include "util/logger.hpp"

World::World():
    _win(sf::VideoMode(800, 600), "mazerun", sf::Style::Default, sf::ContextSettings(24, 8, 8)),
    _running(true), _focused(true), _do_resize(false),
    _sunlight(true, glm::vec3(1.0f, 1.0f, 1.0f), true, glm::normalize(glm::vec3(-1.0f))),
    // TODO: get rid of unused shader files
    _ent_prepass({std::make_pair("shaders/prepass.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/prepass.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0), std::make_pair("vert_tex_coords", 1),
        std::make_pair("vert_normals", 2), std::make_pair("vert_tangents", 3)}),
    _point_light_prog({std::make_pair("shaders/lighting.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/point_light.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)},
        {std::make_pair("diffuse", 0), std::make_pair("specular", 1)}),
    _point_light_shadow_prog({std::make_pair("shaders/lighting.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/point_light_shadow.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)},
        {std::make_pair("diffuse", 0), std::make_pair("specular", 1)}),
    _spot_light_prog({std::make_pair("shaders/lighting.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/spot_light.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)},
        {std::make_pair("diffuse", 0), std::make_pair("specular", 1)}),
    _spot_light_shadow_prog({std::make_pair("shaders/lighting.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/spot_light_shadow.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)},
        {std::make_pair("diffuse", 0), std::make_pair("specular", 1)}),
    _dir_light_prog({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER), // TODO: should we use lighting.vert here?
        std::make_pair("shaders/dir_light.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)},
        {std::make_pair("diffuse", 0), std::make_pair("specular", 1)}),
    _dir_light_shadow_prog({std::make_pair("shaders/lighting.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/dir_light_shadow.frag", GL_FRAGMENT_SHADER),
        std::make_pair("shaders/lighting.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)},
        {std::make_pair("diffuse", 0), std::make_pair("specular", 1)}),
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
    _fullscreen_tex({std::make_pair("shaders/pass-through.vert", GL_VERTEX_SHADER), // TODO: not needed
        std::make_pair("shaders/just-texture.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)}),
    _g_fbo_norm_shininess_tex(FBO::create_color_tex(800, 600, true)), // TODO: resize
    _g_fbo_depth_tex(FBO::create_depth_tex(800, 600)),
    _diffuse_fbo_tex(FBO::create_color_tex(800, 600, false)),
    _specular_fbo_tex(FBO::create_color_tex(800, 600, false)),
    _point_shadow_fbo_tex(FBO::create_shadow_cube_tex(512, 512)),
    _point_shadow_fbo_depth_rbo(Renderbuffer::create_depth(512, 512)),
    _spot_dir_shadow_fbo_tex(FBO::create_shadow_tex(512, 512)),
    _font("Symbola", 18),
    _s_text(_font, u8"🐙💩☹☢☣☠\u0301\nASDF‽", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f))
{
    // TODO: standardize naming
    Logger_locator::get()(Logger::TRACE, "World init starting...");
    // TODO: loading screen
    _ents.emplace_back(create_player());
    _ents.emplace_back(create_testmdl());
    _ents.emplace_back(create_testlight());
    _ents.emplace_back(create_testmonkey());
    _ents.emplace_back(create_testdoughnut());
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

    // TODO: organize texture attachments to reduce swapping

    // Uniform setup
    _ent_prepass.use();
    glUniform1i(_ent_prepass.get_uniform("material.shininess_map"), 0);
    glUniform1i(_ent_prepass.get_uniform("material.normal_map"), 1);

    _point_light_prog.use();
    glUniform1i(_point_light_prog.get_uniform("norm_shininess_map"), 0);
    glUniform1i(_point_light_prog.get_uniform("depth_map"), 1);
    glUniform3fv(_point_light_prog.get_uniform("cam_light_forward"), 1, &cam_light_forward[0]);

    _point_light_shadow_prog.use();
    glUniform1i(_point_light_shadow_prog.get_uniform("norm_shininess_map"), 0);
    glUniform1i(_point_light_shadow_prog.get_uniform("depth_map"), 1);
    glUniform1i(_point_light_shadow_prog.get_uniform("shadow_map"), 2);
    glUniform3fv(_point_light_shadow_prog.get_uniform("cam_light_forward"), 1, &cam_light_forward[0]);

    _spot_light_prog.use();
    glUniform1i(_spot_light_prog.get_uniform("norm_shininess_map"), 0);
    glUniform1i(_spot_light_prog.get_uniform("depth_map"), 1);
    glUniform3fv(_spot_light_prog.get_uniform("cam_light_forward"), 1, &cam_light_forward[0]);

    _spot_light_shadow_prog.use();
    glUniform1i(_spot_light_shadow_prog.get_uniform("norm_shininess_map"), 0);
    glUniform1i(_spot_light_shadow_prog.get_uniform("depth_map"), 1);
    glUniform1i(_spot_light_shadow_prog.get_uniform("shadow_map"), 2);
    glUniform3fv(_spot_light_shadow_prog.get_uniform("cam_light_forward"), 1, &cam_light_forward[0]);

    _dir_light_prog.use();
    glUniform1i(_dir_light_prog.get_uniform("norm_shininess_map"), 0);

    _dir_light_shadow_prog.use();
    glUniform1i(_dir_light_shadow_prog.get_uniform("norm_shininess_map"), 0);
    glUniform1i(_dir_light_shadow_prog.get_uniform("depth_map"), 1);
    glUniform1i(_dir_light_shadow_prog.get_uniform("shadow_map"), 2);

    _set_depth_prog.use();
    glUniform1i(_set_depth_prog.get_uniform("tex"), 0);

    _ent_shader.use();
    glUniform1i(_ent_shader.get_uniform("material.ambient_map"), 0);
    glUniform1i(_ent_shader.get_uniform("material.diffuse_map"), 1);
    glUniform1i(_ent_shader.get_uniform("material.specular_map"), 2);
    glUniform1i(_ent_shader.get_uniform("material.emissive_map"), 3);
    glUniform1i(_ent_shader.get_uniform("material.reflectivity_map"), 4);
    glUniform1i(_ent_shader.get_uniform("env_map"), 5);
    glUniform1i(_ent_shader.get_uniform("diffuse_fbo_tex"), 6);
    glUniform1i(_ent_shader.get_uniform("specular_fbo_tex"), 7);
    glUniform1i(_ent_shader.get_uniform("norm_shininess_map"), 8);

    // TODO: remove
    _fullscreen_tex.use();
    glUniform1i(_fullscreen_tex.get_uniform("tex"), 0);

    glUseProgram(0); // TODO get prev val?

    // setup FBOs
    _g_fbo.bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _g_fbo_norm_shininess_tex->get_id(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _g_fbo_depth_tex->get_id(), 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    _g_fbo.verify();

    _lighting_fbo.bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _diffuse_fbo_tex->get_id(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _specular_fbo_tex->get_id(), 0);
    const GLenum buffs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffs);

    _lighting_fbo.verify();

    _point_shadow_fbo.bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, _point_shadow_fbo_tex->get_id(), 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _point_shadow_fbo_depth_rbo->get_id());
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    _point_shadow_fbo.verify();

    _spot_dir_shadow_fbo.bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _spot_dir_shadow_fbo_tex->get_id(), 0);
    glDrawBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Message_locator::get().add_callback_empty("sun_toggle", [this](){ _sunlight.enabled = !_sunlight.enabled; });

    Shader_prog::clear_cache();

    check_error("World::World");
}
