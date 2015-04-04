// entity.cpp
// main entity class

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

#include "world/entity.hpp"

#include <utility>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/logger.hpp"

Entity::Entity(std::shared_ptr<Model> model,
    Input * input,
    Physics * physics,
    Light * light,
    Audio * audio):
    _model(model),
    _input(input),
    _physics(physics),
    _light(light),
    _audio(audio)
{
    set();
}

std::shared_ptr<Model> Entity::model()
{
    return _model;
}

Input * Entity::input()
{
    return _input.get();
}

Physics * Entity::physics()
{
    return _physics.get();
}

Light * Entity::light()
{
    return _light.get();
}

Audio * Entity::audio()
{
    return _audio.get();
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
    _pos = pos;
}

void Entity::set_facing(const glm::vec3 & forward, const glm::vec3 & up)
{
    glm::vec3 norm_forward = glm::normalize(forward);
    glm::vec3 norm_up = glm::normalize(up);
    glm::vec3 norm_right = glm::cross(norm_forward, norm_up);

    _rot = glm::quat(glm::mat3(norm_right, norm_up, -norm_forward));
}

// move the position acoording to a vector
void Entity::translate(const glm::vec3 & translation)
{
    _pos += translation;
}

// rotate around an axis (in world space)
// angles in radians
void Entity::rotate_world(const float angle, const glm::vec3 & axis)
{
    _rot = glm::normalize(glm::angleAxis(angle, glm::normalize(axis)) * _rot);
}

void Entity::rotate_local(const float angle, const glm::vec3 & axis)
{
    _rot = glm::normalize(_rot * glm::angleAxis(angle, glm::normalize(axis)));
}

glm::mat4 Entity::view_mat() const
{
    return glm::inverse(model_mat());
}

glm::mat4 Entity::model_mat() const
{
    glm::mat4 mdl(glm::mat3_cast(_rot));
    mdl[3] = glm::vec4(_pos, 1.0f);
    return mdl;
}

glm::vec3 Entity::pos() const
{
    return _pos;
}

// we get component vectors by doing only the relevant part of the quat->matrix conversion
glm::vec3 Entity::forward() const
{
    return -glm::vec3(
        2.0f * _rot.x * _rot.z + 2.0f * _rot.y * _rot.w,
        2.0f * _rot.y * _rot.z - 2.0f * _rot.x * _rot.w,
        1.0f - 2.0f * _rot.x * _rot.x - 2.0f * _rot.y * _rot.y);
}

glm::vec3 Entity::up() const
{
    return glm::vec3(
        2.0f * _rot.x * _rot.y - 2.0f * _rot.z * _rot.w,
        1.0f - 2.0f * _rot.x * _rot.x - 2.0f * _rot.z * _rot.z,
        2.0f * _rot.y * _rot.z + 2.0f * _rot.x * _rot.w);
}

glm::vec3 Entity::right() const
{
    return glm::vec3(
        1.0f - 2.0f * _rot.y * _rot.y - 2.0f * _rot.z * _rot.z,
        2.0f * _rot.x * _rot.y + 2.0f * _rot.z * _rot.w,
        2.0f * _rot.x * _rot.z - 2.0f * _rot.y * _rot.w);
}
