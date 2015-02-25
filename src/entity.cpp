// entity.cpp
// entity base class

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

#include "entity.hpp"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

Entity::Entity()
{
    set();
}

// set / reset pos / orientation
void Entity::set(const glm::vec3 & pos, const glm::vec3 & forward,
    const glm::vec3 & up)
{
    set_pos(pos);
    set_facing(forward, up);
}

void Entity::set_pos(const glm::vec3 & pos)
{
    _model_mat[3] = glm::vec4(pos, 1.0f);
}

void Entity::set_facing(const glm::vec3 & forward, const glm::vec3 & up)
{
    glm::vec3 norm_forward = glm::normalize(forward);
    glm::vec3 norm_up = glm::normalize(up);
    glm::vec3 norm_right = glm::cross(norm_forward, norm_up);

    _model_mat[0] = glm::vec4(norm_right, 0.0f);
    _model_mat[1] = glm::vec4(norm_up, 0.0f);
    _model_mat[2] = glm::vec4(-norm_forward, 0.0f);
}

// move the position acoording to a vector
void Entity::translate(const glm::vec3 & translation)
{
    set_pos(pos() + translation);
}

// rotate around an axis (in world space)
// angles in radians
void Entity::rotate(const float angle, const glm::vec3 & axis)
{
    // using Rodrigues' rotation formula
    // http://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
    glm::vec3 new_forward = forward() * cosf(angle) + glm::cross(axis, forward()) * sinf(angle)
        + axis * glm::dot(axis, forward()) * (1.0f - cosf(angle));
    glm::vec3 new_up = up() * cosf(angle) + glm::cross(axis, up()) * sinf(angle)
        + axis * glm::dot(axis, up()) * (1.0f - cosf(angle));
    set_facing(new_forward, new_up);
}

glm::mat4 Entity::view_mat() const
{
    return glm::lookAt(pos(), pos() + forward(), up());
}

glm::mat4 Entity::model_mat() const
{
    return _model_mat;
}

glm::vec3 Entity::pos() const
{
    return glm::vec3(_model_mat[3]);
}

glm::vec3 Entity::forward() const
{
    return -glm::normalize(glm::vec3(_model_mat[2]));
}

glm::vec3 Entity::up() const
{
    return glm::normalize(glm::vec3(_model_mat[1]));
}

glm::vec3 Entity::right() const
{
    return glm::normalize(glm::vec3(_model_mat[0]));
}
