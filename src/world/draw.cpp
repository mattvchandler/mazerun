// draw.cpp
// main drawing

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

#include <chrono>
#include <iomanip>
#include <sstream>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#ifdef DEBUG
#include "opengl/gl_helpers.hpp"
#endif

#include "util/logger.hpp"

// TODO: weird lighting artifacts
void World::draw()
{
    // TODO: no lighting until camera moved, and when pointed down
    const glm::vec3 cam_light_forward(0.0f, 0.0f, 1.0f); // in eye space
    glm::vec2 win_size(_win.getSize().x, _win.getSize().y);

    // TODO: max on lighting, shadows?
    std::vector<Entity *> point_lights;
    std::vector<Entity *> spot_lights;
    std::vector<Entity *> models;
    point_lights.reserve(_ents.size());
    spot_lights.reserve(_ents.size());
    models.reserve(_ents.size());

    auto set_prepass_material = [this](const Material & mat)
    {
        glUniform1f(_ent_prepass.get_uniform("material.shininess"), mat.shininess);
        glActiveTexture(GL_TEXTURE5);
        mat.normal_shininess_map->bind();
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
            glUniformMatrix3fv(_ent_prepass.get_uniform("normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

            model->draw(set_prepass_material);

            #ifdef DEBUG
            check_error("World::draw - prepass");
            #endif
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

    glClear(GL_COLOR_BUFFER_BIT);

    _point_light_shadow_prog.use();
    glUniform1f(_point_light_shadow_prog.get_uniform("aspect"), win_size.x / win_size.y);
    glUniform1f(_point_light_shadow_prog.get_uniform("tan_half_fov"), std::tan(M_PI / 12.0f));
    glUniformMatrix4fv(_point_light_shadow_prog.get_uniform("proj_mat"), 1, GL_FALSE, &_proj[0][0]);
    glUniform2fv(_point_light_shadow_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

    _point_light_prog.use();
    glUniform1f(_point_light_prog.get_uniform("aspect"), win_size.x / win_size.y);
    glUniform1f(_point_light_prog.get_uniform("tan_half_fov"), std::tan(M_PI / 12.0f));
    glUniformMatrix4fv(_point_light_prog.get_uniform("proj_mat"), 1, GL_FALSE, &_proj[0][0]);
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

                    #ifdef DEBUG
                    check_error("World::draw - spot light shadow map");
                    #endif
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

            #ifdef DEBUG
            check_error("World::draw - point light shadow quad");
            #endif

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

            #ifdef DEBUG
            check_error("World::draw - point light quad");
            #endif
        }
    }

    glPolygonOffset(2.0f, 4.0f);

    _spot_light_shadow_prog.use();
    glUniform1f(_spot_light_shadow_prog.get_uniform("aspect"), win_size.x / win_size.y);
    glUniform1f(_spot_light_shadow_prog.get_uniform("tan_half_fov"), std::tan(M_PI / 12.0f));
    glUniformMatrix4fv(_spot_light_shadow_prog.get_uniform("proj_mat"), 1, GL_FALSE, &_proj[0][0]);
    glUniform2fv(_spot_light_shadow_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

    _spot_light_prog.use();
    glUniform1f(_spot_light_prog.get_uniform("aspect"), win_size.x / win_size.y);
    glUniform1f(_spot_light_prog.get_uniform("tan_half_fov"), std::tan(M_PI / 12.0f));
    glUniformMatrix4fv(_spot_light_prog.get_uniform("proj_mat"), 1, GL_FALSE, &_proj[0][0]);
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

                #ifdef DEBUG
                check_error("World::draw - spot light shadow map");
                #endif
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

            #ifdef DEBUG
            check_error("World::draw - spot light shadow quad");
            #endif

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

            #ifdef DEBUG
            check_error("World::draw - spot light quad");
            #endif
        }
    }

    auto dir_common = [this, &cam_light_forward, &viewport_size](const Shader_prog & dir_prog)
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

                #ifdef DEBUG
                check_error("World::draw - dir light shadow map");
                #endif
            }

            _lighting_fbo.bind();
            glViewport(0, 0, 800, 600);
            _dir_light_shadow_prog.use();
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_POLYGON_OFFSET_FILL);

            glUniform1f(_dir_light_shadow_prog.get_uniform("aspect"), win_size.x / win_size.y);
            glUniform1f(_dir_light_shadow_prog.get_uniform("tan_half_fov"), std::tan(M_PI / 12.0f));
            glUniformMatrix4fv(_dir_light_shadow_prog.get_uniform("proj_mat"), 1, GL_FALSE, &_proj[0][0]);
            glUniformMatrix4fv(_dir_light_shadow_prog.get_uniform("shadow_mat"), 1, GL_FALSE, &dir_shadow_mat[0][0]);
            dir_common(_dir_light_shadow_prog);

            #ifdef DEBUG
            check_error("World::draw - dir light shadow quad");
            #endif
        }
        else
        {
            _dir_light_prog.use();
            dir_common(_dir_light_prog);

            #ifdef DEBUG
            check_error("World::draw - dir light quad");
            #endif
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, win_size.x, win_size.y);
    viewport_size = win_size;

    // set default framebuffer depth buffer from G buffer
    glDisable(GL_BLEND);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _set_depth_prog.use();
    glUniform2fv(_set_depth_prog.get_uniform("viewport_size"), 1, &viewport_size[0]);

    _quad.draw();

    #ifdef DEBUG
    check_error("World::draw - default depthbuffer fill");
    #endif

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
        glUniform1f(_ent_shader.get_uniform("material.reflectivity"), mat.reflectivity);

        glActiveTexture(GL_TEXTURE1);
        mat.ambient_map->bind();
        glActiveTexture(GL_TEXTURE2);
        mat.diffuse_map->bind();
        glActiveTexture(GL_TEXTURE3);
        mat.specular_map->bind();
        glActiveTexture(GL_TEXTURE4);
        mat.emissive_reflectivity_map->bind();
    };

    glm::mat3 inv_view = glm::mat3(_cam->model_mat());

    glUniform2fv(_ent_shader.get_uniform("viewport_size"), 1, &viewport_size[0]);
    glUniformMatrix3fv(_ent_shader.get_uniform("inv_view"), 1, GL_FALSE, &inv_view[0][0]);

    for(auto & ent: models)
    {
        Model * model = ent->model();

        glm::mat4 model_view = _cam->view_mat() * ent->model_mat();
        glm::mat4 model_view_proj = _proj * model_view;

        glUniformMatrix4fv(_ent_shader.get_uniform("model_view"), 1, GL_FALSE, &model_view[0][0]);
        glUniformMatrix4fv(_ent_shader.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);

        model->draw(set_material);

    }

    #ifdef DEBUG
    check_error("World::draw - main pass");
    #endif

    glDisable(GL_POLYGON_OFFSET_FILL);

    _skybox.draw(*_cam, _proj);

    // TODO: antialiasing

    _s_text.render_text(_font, win_size, glm::vec2(10.0f, 10.0f),
        Font_sys::ORIGIN_HORIZ_LEFT | Font_sys::ORIGIN_VERT_TOP);

    static int frame_count = 0;
    static float fps = 0.0f;
    if(frame_count++ >= 10)
    {
        static auto last_frame = std::chrono::high_resolution_clock::now();
        auto now =  std::chrono::high_resolution_clock::now();
        fps = (float)frame_count / std::chrono::duration<float, std::ratio<1, 1>>(now - last_frame).count();
        last_frame = now;
        frame_count = 0;
    }

    static std::ostringstream fps_format;
    fps_format.str("");
    fps_format<<std::setprecision(3)<<std::fixed<<fps<<" fps";
    _font.render_text(fps_format.str(), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), win_size,
        glm::vec2(win_size.x - 10.0f, 10.0f), Font_sys::ORIGIN_HORIZ_RIGHT | Font_sys::ORIGIN_VERT_TOP);

    _win.display();

    #ifdef DEBUG
    check_error("World::draw - end");
    #endif
}
