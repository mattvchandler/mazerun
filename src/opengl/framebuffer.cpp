// framebuffer.cpp
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

#include "opengl/framebuffer.hpp"

#include "opengl/gl_helpers.hpp"
#include "util/logger.hpp"

Framebuffer::~Framebuffer()
{
    Logger_locator::get()(Logger::TRACE, "Deleting GL FBO: " + std::to_string(_id));
    glDeleteFramebuffers(1, &_id);
}

void Framebuffer::bind_fbo() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

GLuint Framebuffer::get_fbo_id() const
{
    return _id;
}

GLuint Framebuffer::get_width() const
{
    return _width;
}

GLuint Framebuffer::get_height() const
{
    return _height;
}

std::string Framebuffer::error_string(GLenum error)
{
    switch(error)
    {
    case GL_FRAMEBUFFER_COMPLETE:
        return "Complete framebuffer - no error";
    case GL_FRAMEBUFFER_UNDEFINED:
        return "Undefined framebuffer";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        return "Incomplete attachment - a necessary attachment is uninitialized";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        return "Missing attachment - no image attached";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        return "Every drawing buffer has an attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        return "Attachment exists for read buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED:
        return "Unsupported attachment combination";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        return "Number of samples does not match across attachments";
    default:
        return "Unknown framebuffer status";
    }
}

Framebuffer::Framebuffer(const GLuint width, const GLuint height):
    _width(width), _height(height)
{
    glGenFramebuffers(1, &_id);
    Logger_locator::get()(Logger::TRACE, "Generated GL FBO: " + std::to_string(_id));
}

Shadow_FBO::Shadow_FBO(const GLuint width, const GLuint height):
    Framebuffer(width, height)
{
    _tex = std::unique_ptr<Texture>(new Texture_2D);
    bind_tex();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _width, _height, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glBindTexture(GL_TEXTURE_2D, 0);

    bind_fbo();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, get_tex_id(), 0);
    glDrawBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::string error = error_string(status);
        Logger_locator::get()(Logger::WARN, "Incomplete framebuffer: " + error);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    check_error("Shadow_FBO::Shadow_FBO");
}

void Shadow_FBO::bind_tex() const
{
    _tex->bind();
}

GLuint Shadow_FBO::get_tex_id() const
{
    return _tex->get_id();
}

G_FBO::G_FBO(const GLuint width, const GLuint height):
    Framebuffer(width, height)
{
    _pos_tex = std::unique_ptr<Texture>(new Texture_2D);
    _shininess_tex = std::unique_ptr<Texture>(new Texture_2D);
    _normal_tex = std::unique_ptr<Texture>(new Texture_2D);
    _depth_tex = std::unique_ptr<Texture>(new Texture_2D);

    bind_fbo();

    int i = 0;
    for(auto tex: {get_pos_tex_id(), get_shininess_tex_id(), get_normal_tex_id()})
    {
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _width, _height, 0,
            GL_RGB, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (i++), GL_TEXTURE_2D, tex, 0);
    }

    _depth_tex->bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _width, _height, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, get_depth_tex_id(), 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    const GLenum buffs[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, buffs);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::string error = error_string(status);
        Logger_locator::get()(Logger::WARN, "Incomplete framebuffer: " + error);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    check_error("Shadow_FBO::Shadow_FBO");
}

void G_FBO::bind_pos_tex() const
{
    _pos_tex->bind();
}

void G_FBO::bind_shininess_tex() const
{
    _shininess_tex->bind();
}

void G_FBO::bind_normal_tex() const
{
    _normal_tex->bind();
}

void G_FBO::bind_depth_tex() const
{
    _depth_tex->bind();
}

GLuint G_FBO::get_pos_tex_id() const
{
    return _pos_tex->get_id();
}

GLuint G_FBO::get_shininess_tex_id() const
{
    return _shininess_tex->get_id();
}

GLuint G_FBO::get_normal_tex_id() const
{
    return _normal_tex->get_id();
}

GLuint G_FBO::get_depth_tex_id() const
{
    return _depth_tex->get_id();
}
