// light.cpp
// light structures

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

// TODO: remove virtual dtors as able

#include "components/light.hpp"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

Light::Light(const bool enabled, const glm::vec3 & color, const bool casts_shadow):
    enabled(enabled), color(color), casts_shadow(casts_shadow)
{
}

Point_light::Point_light(const bool enabled, const glm::vec3 & color, const bool casts_shadow,
    const glm::vec3 & pos, const float const_atten, const float linear_atten,
    const float quad_atten):
    Light(enabled, color, casts_shadow), pos(pos),
    const_atten(const_atten), linear_atten(linear_atten), quad_atten(quad_atten)
{
}

std::shared_ptr<Point_light> Point_light::create(const bool enabled, const glm::vec3 & color,
    const bool casts_shadow, const glm::vec3 & pos, const float const_atten,
    const float linear_atten, const float quad_atten)
{
    return std::make_shared<Point_light>(enabled, color, casts_shadow, pos, const_atten,
        linear_atten, quad_atten);
}

glm::mat4 Point_light::shadow_view_mat(const glm::vec3 & axis)
{
    return glm::mat4(1.0f);
}

glm::mat4 Point_light::shadow_proj_mat(const glm::vec3 & axis)
{
    return glm::mat4(1.0f);
}

Spot_light::Spot_light(const bool enabled, const glm::vec3 & color, const bool casts_shadow,
    const glm::vec3 & pos, const glm::vec3 & dir, const float cos_cutoff,
    const float exponent, const float const_atten, const float linear_atten,
    const float quad_atten):
    Light(enabled, color, casts_shadow), pos(pos), dir(dir), cos_cutoff(cos_cutoff),
    exponent(exponent), const_atten(const_atten), linear_atten(linear_atten), quad_atten(quad_atten)
{
    shadow_fbo = std::unique_ptr<Shadow_FBO>(new Shadow_FBO(512)); // get the size from some constant, possibly config
}

std::shared_ptr<Spot_light> Spot_light::create(const bool enabled, const glm::vec3 & color,
    const bool casts_shadow, const glm::vec3 & pos, const glm::vec3 & dir,
    const float cos_cutoff, const float exponent, const float const_atten,
    const float linear_atten, const float quad_atten)
{
    return std::make_shared<Spot_light>(enabled, color, casts_shadow, pos, dir,
        cos_cutoff, exponent, const_atten, linear_atten, quad_atten);
}

glm::mat4 Spot_light::shadow_view_mat()
{
    // find an orthographic vector
    glm::vec3 up = glm::normalize(glm::vec3(dir.y, -dir.x, 0.0f) + glm::vec3(-dir.z, 0.0f, dir.x));

    return glm::lookAt(pos, pos + dir, up);
}

glm::mat4 Spot_light::shadow_proj_mat()
{
    return glm::perspective(2.0f * std::acos(cos_cutoff), 1.0f, 0.1f, 100.0f);
}

Dir_light::Dir_light(const bool enabled, const glm::vec3 & color, const bool casts_shadow,
    const glm::vec3 & dir):
    Light(enabled, color, casts_shadow), dir(dir)
{
}

glm::mat4 Dir_light::shadow_view_mat()
{
    return glm::mat4(1.0f);
}

glm::mat4 Dir_light::shadow_proj_mat()
{
    return glm::mat4(1.0f);
}
