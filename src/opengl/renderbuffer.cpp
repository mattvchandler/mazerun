// renderbuffer.cpp
// Renderbuffer object

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

#include "opengl/renderbuffer.hpp"

#include <string>

#include "opengl/gl_helpers.hpp"
#include "util/logger.hpp"

Renderbuffer::~Renderbuffer()
{
    Logger_locator::get()(Logger::TRACE, "Deleting GL Renderbuffer: " + std::to_string(_id));
    glDeleteRenderbuffers(1, &_id);
}

void Renderbuffer::bind() const
{
    glBindRenderbuffer(GL_RENDERBUFFER, _id);
}

GLuint Renderbuffer::get_id() const
{
    return _id;
}

Renderbuffer * Renderbuffer::create_color(const GLuint width, const GLuint height, const bool rgba)
{
    Renderbuffer * rbo = new Renderbuffer;
    rbo->bind();

    if(rgba)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, width, height);

    check_error("Renderbuffer::create_color");
    return rbo;
}

Renderbuffer * Renderbuffer::create_depth(const GLuint width, const GLuint height)
{
    Renderbuffer * rbo = new Renderbuffer;
    rbo->bind();

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    check_error("Renderbuffer::create_depth");

    return rbo;
}

Renderbuffer::Renderbuffer()
{
    glGenRenderbuffers(1, &_id);
    Logger_locator::get()(Logger::TRACE, "Generated GL Renderbuffer: " + std::to_string(_id));
}
