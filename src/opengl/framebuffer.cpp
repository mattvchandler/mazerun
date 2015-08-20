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

FBO::FBO()
{
    glGenFramebuffers(1, &_id);
    Logger_locator::get()(Logger::TRACE, "Generated GL FBO: " + std::to_string(_id));
}

FBO::~FBO()
{
    Logger_locator::get()(Logger::TRACE, "Deleting GL FBO: " + std::to_string(_id));
    glDeleteFramebuffers(1, &_id);
}

void FBO::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

GLuint FBO::get_id() const
{
    return _id;
}

bool FBO::verify() const
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::string error = error_string(status);
        Logger_locator::get()(Logger::WARN, "Incomplete framebuffer: " + error);
        return false;
    }
    return true;
}

std::string FBO::error_string(GLenum error)
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

Texture_2D * FBO::create_color_tex(const GLuint width, const GLuint height, const GLenum internal_format)
{
    Texture_2D * tex = new Texture_2D;
    tex->bind();

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

Texture_2D * FBO::create_depth_tex(const GLuint width, const GLuint height)
{
    Texture_2D * tex = new Texture_2D;
    tex->bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

Texture_2D * FBO::create_shadow_tex(const GLuint width, const GLuint height)
{
    Texture_2D * tex = new Texture_2D;
    tex->bind();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0,
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
    return tex;
}

Texture_cubemap * FBO::create_shadow_cube_tex(const GLuint width, const GLuint height)
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // TODO: should this be enabled at higher scope?
    Texture_cubemap * tex = new Texture_cubemap;
    tex->bind();

    std::vector<GLenum> sides =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    };

    for(const auto & side: sides)
    {
        glTexImage2D(side, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return tex;
}
