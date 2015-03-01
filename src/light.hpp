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
    virtual ~Light();
    glm::vec3 color;
    float strength;
protected:
    Light(const glm::vec3 & color, const float strength);
};

class Point_light: public Light
{
public:
    Point_light(const glm::vec3 & color, const float strength, const glm::vec3 & pos,
        const float const_atten, const float linear_atten, const float quad_atten);

    static std::shared_ptr<Point_light> create(const glm::vec3 & color, const float strength,
        const glm::vec3 & pos, const float const_atten, const float linear_atten,
        const float quad_atten);

    glm::vec3 pos;
    // attenuation properties
    float const_atten;
    float linear_atten;
    float quad_atten;
};

class Dir_light: public Light
{
public:
    Dir_light(const glm::vec3 & color, const float strength, const glm::vec3 & dir);

    glm::vec3 dir;
};

// TODO: spotlight

#endif // LIGHT_HPP
