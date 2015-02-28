// testmdl.cpp
// test entity

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

#include "testmdl.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <glm/glm.hpp>

#include "entity.hpp"

std::shared_ptr<Testmdl_physics> Testmdl_physics::create()
{
    return std::shared_ptr<Testmdl_physics>(new Testmdl_physics());
}

void Testmdl_physics::update(Entity & ent,
    const float dt) const
{
    ent.rotate(dt * 0.5f * M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
}

Testmdl_physics::Testmdl_physics()
{
}

Entity create_testmdl()
{
    Entity testmdl(Model::create("mdl/weird_cube.dae"),
        std::shared_ptr<Input>(),
        Testmdl_physics::create());

    testmdl.set_pos(glm::vec3(0.0f, 5.0f, 0.0f));
    testmdl.rotate(M_PI / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    testmdl.rotate(M_PI / 8.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    return testmdl;
}
