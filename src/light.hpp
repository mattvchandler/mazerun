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

struct Light
{
    glm::vec3 color;
    float strength;
};

struct Point_light: public Light
{
    glm::vec3 pos;
    // attenuation properties
    float const_atten;
    float linear_atten;
    float quad_atten;
    Point_light(const glm::vec3 & color, const float strength, const glm::vec3 & pos,
    const float const_atten, const float linear_atten, const float quad_atten):
        Light({color, strength}), pos(pos),
        const_atten(const_atten), linear_atten(linear_atten), quad_atten(quad_atten)
    {
    }
};

struct Dir_light: public Light
{
    glm::vec3 dir;
    Dir_light(const glm::vec3 & color, const float strength, const glm::vec3 & dir):
        Light({color, strength}), dir(dir)
        {
    }
};

// TODO: spotlight

#endif // LIGHT_HPP
