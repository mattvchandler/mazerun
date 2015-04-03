// framebuffer.hpp
// Framebuffer object

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

#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>

#include "opengl/texture.hpp"

class Framebuffer: public sf::NonCopyable
{
public:
    virtual ~Framebuffer();
    virtual void bind_fbo() const = 0;
    void bind_tex() const;
    GLuint get_fbo_id() const;
    GLuint get_tex_id() const;

    static std::string error_string(GLenum error);

protected:
    Framebuffer();

    GLuint _id;
    std::unique_ptr<Texture> _tex;
};

class Shadow_FBO final: public Framebuffer
{
public:
    Shadow_FBO(const std::size_t size);
    void bind_fbo() const;
};

#endif // FRAMEBUFFER_HPP