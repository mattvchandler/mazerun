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

#include "gl_wrappers.hpp"

GL_buffer::GL_buffer(const GLenum type):
    _type(type)
{
    glGenBuffers(1, &_buf);
}

GL_buffer::~GL_buffer()
{
    glDeleteBuffers(1, &_buf);
}

void GL_buffer::bind() const
{
    glBindBuffer(_type, _buf);
}

GLenum GL_buffer::type() const
{
    return _type;
}

GLuint GL_buffer::operator ()() const
{
    return _buf;
}

GL_vertex_array::GL_vertex_array()
{
    glGenVertexArrays(1, &_arr);
}

GL_vertex_array::~GL_vertex_array()
{
    glDeleteVertexArrays(1, &_arr);
}

void GL_vertex_array::bind() const
{
    glBindVertexArray(_arr);
}

GLuint GL_vertex_array::operator ()() const
{
    return _arr;
}
