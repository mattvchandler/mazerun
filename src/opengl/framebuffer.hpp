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

class FBO final: public sf::NonCopyable
{
public:
    FBO();
    virtual ~FBO();
    virtual void bind() const;
    GLuint get_id() const;
    bool verify() const;

    static std::string error_string(GLenum error);

    static Texture_2D * create_tex(const GLuint width, const GLuint height, const bool rgba = true);
    static Texture_2D * create_depth_tex(const GLuint width, const GLuint height);
    static Texture_2D * create_shadow_tex(const GLuint width, const GLuint height);
    static Texture_cubemap * create_shadow_cube_tex(const GLuint width, const GLuint height);

private:

    GLuint _id;
};

// TODO: renderbuffer class

#endif // FRAMEBUFFER_HPP
