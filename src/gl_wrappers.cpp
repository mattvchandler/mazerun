// gl_wrappers.hpp
// RAII wrappers for open GL objects

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

#include <stdexcept>
#include <system_error>

#include "gl_wrappers.hpp"

GL_buffer::GL_buffer(const GLenum type):
    _buf(0), _type(type)
{
}

GL_buffer::~GL_buffer()
{
    if(_buf)
        glDeleteBuffers(1, &_buf);
}

void GL_buffer::gen()
{
    if(_buf)
        throw std::runtime_error("Attempt to regen initialized buffer");

    glGenBuffers(1, &_buf);
}

void GL_buffer::bind() const
{
    if(_buf)
        glBindBuffer(_type, _buf);
    else
        throw std::runtime_error("Attempt to use uninitialized buffer");
}

GLenum GL_buffer::type() const
{
    if(_buf)
        return _type;
    else
        throw std::runtime_error("Attempt to use uninitialized buffer");
}

GLuint GL_buffer::operator ()() const
{
    return _buf;
}

GL_vertex_array::GL_vertex_array():
    _arr(0)
{
}

GL_vertex_array::~GL_vertex_array()
{
    if(_arr)
        glDeleteVertexArrays(1, &_arr);
}

void GL_vertex_array::gen()
{
    if(_arr)
        throw std::runtime_error("Attempt to regen initialized array");

    glGenVertexArrays(1, &_arr);
}

void GL_vertex_array::bind() const
{
    if(_arr)
        glBindVertexArray(_arr);
    else
        throw std::runtime_error("Attempt to use uninitialized array");
}

GLuint GL_vertex_array::operator ()() const
{
    if(_arr)
        return _arr;
    else
        throw std::runtime_error("Attempt to use uninitialized array");
}
