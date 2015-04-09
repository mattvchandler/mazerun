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
    virtual void bind_fbo() const;
    GLuint get_fbo_id() const;

    GLuint get_width() const;
    GLuint get_height() const;

    static std::string error_string(GLenum error);

protected:
    Framebuffer(const GLuint width, const GLuint height);

    GLuint _id;
    GLuint _width, _height;
};

class Shadow_FBO final: public Framebuffer
{
public:
    Shadow_FBO(const GLuint width, const GLuint height);

    void bind_tex() const;
    GLuint get_tex_id() const;

private:
    std::unique_ptr<Texture> _tex;
};

class G_FBO final: public Framebuffer
{
public:
    G_FBO(const GLuint width, const GLuint height);

    void bind_diffuse_tex() const;
    void bind_normal_tex() const;
    void bind_pos_tex() const;
    void bind_tex_coord_tex() const;
    void bind_depth_tex() const;
    GLuint get_diffuse_tex_id() const;
    GLuint get_normal_tex_id() const;
    GLuint get_pos_tex_id() const;
    GLuint get_tex_coord_tex_id() const;
    GLuint get_depth_tex_id() const;

private:
    std::unique_ptr<Texture> _diffuse_tex;
    std::unique_ptr<Texture> _normal_tex;
    std::unique_ptr<Texture> _pos_tex;
    std::unique_ptr<Texture> _tex_coord_tex;
    std::unique_ptr<Texture> _depth_tex;
};

#endif // FRAMEBUFFER_HPP
