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

#ifndef GL_WRAPPERS_HPP
#define GL_WRAPPERS_HPP

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>

class GL_buffer: public sf::NonCopyable
{
public:
    GL_buffer(const GLenum type);
    ~GL_buffer();
    void bind() const;
    GLenum type() const;
    GLuint get_id() const;
private:
    GLuint _buf;
    GLenum _type;
};

class GL_vertex_array: public sf::NonCopyable
{
public:
    GL_vertex_array();
    ~GL_vertex_array();
    void bind() const;
    GLuint get_id() const;
private:
    GLuint _arr;
};

#endif // GL_WRAPPERS_HPP
