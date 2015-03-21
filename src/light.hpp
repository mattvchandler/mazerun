// light.hpp
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

#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>

#include "component.hpp"

class Light: public Component
{
public:
    virtual ~Light() = default;
    bool enabled;
    glm::vec3 color;

    bool casts_shadow;

protected:
    Light(const bool enabled, const glm::vec3 & color, const bool casts_shadow);
};

class Point_light: public Light
{
public:
    Point_light(const bool enabled, const glm::vec3 & color, const bool casts_shadow,
        const glm::vec3 & pos, const float const_atten, const float linear_atten,
        const float quad_atten);
    virtual ~Point_light() = default;

    static std::shared_ptr<Point_light> create(const bool enabled, const glm::vec3 & color,
        const bool casts_shadow, const glm::vec3 & pos, const float const_atten,
        const float linear_atten, const float quad_atten);

    glm::vec3 pos;
    // attenuation properties
    float const_atten;
    float linear_atten;
    float quad_atten;
};

class Spot_light:public Light
{
public:
    Spot_light(const bool enabled, const glm::vec3 & color, const bool casts_shadow,
        const glm::vec3 & pos, const glm::vec3 & dir, const float cos_cutoff,
        const float exponent, const float const_atten, const float linear_atten,
        const float quad_atten);

    static std::shared_ptr<Spot_light> create(const bool enabled, const glm::vec3 & color,
        const bool casts_shadow, const glm::vec3 & pos, const glm::vec3 & dir,
        const float cos_cutoff, const float exponent, const float const_atten,
        const float linear_atten, const float quad_atten);

    glm::vec3 pos;
    glm::vec3 dir;
    float cos_cutoff;
    float exponent;
    // attenuation properties
    float const_atten;
    float linear_atten;
    float quad_atten;
};

class Dir_light: public Light
{
public:
    Dir_light(const bool enabled, const glm::vec3 & color, const bool casts_shadow,
        const glm::vec3 & dir);
    virtual ~Dir_light() = default;

    glm::vec3 dir;
};

#endif // LIGHT_HPP
