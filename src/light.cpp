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

#include "light.hpp"

Light::~Light()
{
}

Light::Light(const bool enabled, const glm::vec3 & color, const float strength):
    enabled(enabled), color(color), strength(strength)
{
}

Point_light::Point_light(const bool enabled, const glm::vec3 & color, const float strength,
    const glm::vec3 & pos, const float const_atten, const float linear_atten, const float quad_atten):
    Light(enabled, color, strength), pos(pos),
    const_atten(const_atten), linear_atten(linear_atten), quad_atten(quad_atten)
{
}

Point_light::~Point_light()
{
}

std::shared_ptr<Point_light> Point_light::create(const bool enabled, const glm::vec3 & color,
    const float strength, const glm::vec3 & pos, const float const_atten, const float linear_atten,
    const float quad_atten)
{
    return std::make_shared<Point_light>(enabled, color, strength, pos, const_atten, linear_atten, quad_atten);
}

Dir_light::Dir_light(const bool enabled, const glm::vec3 & color, const float strength,
    const glm::vec3 & dir):
    Light(enabled, color, strength), dir(dir)
{
}

Dir_light::~Dir_light()
{
}
