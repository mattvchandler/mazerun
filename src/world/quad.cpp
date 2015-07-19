// quad.cpp
// quad (for full-screen frag-shading)

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

#include "quad.hpp"

#include <vector>

#include <glm/glm.hpp>

#include "opengl/gl_helpers.hpp"
#include "util/logger.hpp"

Quad::Quad(): _vbo(GL_ARRAY_BUFFER)
{
    Logger_locator::get()(Logger::DBG, "Creating Quad");

    std::vector<glm::vec3> vert_pos =
    {
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, -1.0f, 0.0f),
        glm::vec3(-1.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f)
    };

    _vao.bind();
    _vbo.bind();

    glBufferData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size(), NULL, GL_STATIC_DRAW);

    glBufferSubData(_vbo.type(), 0, sizeof(glm::vec3) * vert_pos.size(), vert_pos.data());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    check_error("Quad::Quad");
}

void Quad::draw() const
{
    _vao.bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0); // TODO: get prev val?

    check_error("Quad::Draw");
}
