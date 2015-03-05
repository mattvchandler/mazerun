// entity.hpp
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

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <memory>

#include <glm/glm.hpp>

#include "input.hpp"
#include "light.hpp"
#include "model.hpp"
#include "physics.hpp"

class Entity final
{
public:
    // TODO: order?
    Entity(std::shared_ptr<Model> model,
        std::shared_ptr<Input> input,
        std::shared_ptr<Physics> physics,
        std::shared_ptr<Light> light);

    // component getters
    std::shared_ptr<Model> model();
    std::shared_ptr<Input> input();
    std::shared_ptr<Physics> physics();
    std::shared_ptr<Light> light();

    // set / reset pos / orientation
    void set(const glm::vec3 & pos = glm::vec3(0.0f),
        const glm::vec3 & forward = glm::vec3(0.0f, 0.0f, -1.0f),
        const glm::vec3 & up = glm::vec3(0.0f, 1.0f, 0.0f));

    void set_pos(const glm::vec3 & pos = glm::vec3(0.0f));

    void set_facing(const glm::vec3 & forward = glm::vec3(0.0f, 0.0f, -1.0f),
        const glm::vec3 & up = glm::vec3(0.0f, 1.0f, 0.0f));

    // move the position acoording to a vector
    void translate(const glm::vec3 & translation);

    // rotate around an axis
    // angles in radians
    void rotate(const float angle, const glm::vec3 & axis);

    glm::mat4 view_mat() const;
    glm::mat4 model_mat() const;

    glm::vec3 pos() const;
    glm::vec3 forward() const;
    glm::vec3 up() const;
    glm::vec3 right() const;
protected:
    glm::mat4 _model_mat;

    // components
    std::shared_ptr<Model> _model;
    std::shared_ptr<Input> _input;
    std::shared_ptr<Physics> _physics;
    std::shared_ptr<Light> _light;
};

#endif // ENTITY_HPP
