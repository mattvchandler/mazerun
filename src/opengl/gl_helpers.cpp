// gl_helpers.hpp
// common OpenGL utils

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

#include "opengl/gl_helpers.hpp"

#include <sstream>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>

#include "util/logger.hpp"

// check for OpenGL error and print message
void check_error(const std::string & at)
{
    GLenum e = glGetError();
    if(e == GL_NO_ERROR)
        return;

    Logger_locator::get()(Logger::WARN,
        "OpenGL Error at " + at + ": " + (const char *)(gluErrorString(e)));
}

std::ostream & operator<<(std::ostream & out, const glm::vec2 & v)
{
    return out<<"("<<v.x<<","<<v.y<<")";
}

std::ostream & operator<<(std::ostream & out, const glm::vec3 & v)
{
    return out<<"("<<v.x<<","<<v.y<<","<<v.z<<")";
}

std::ostream & operator<<(std::ostream & out, const glm::vec4 & v)
{
    return out<<"("<<v.x<<","<<v.y<<","<<v.z<<","<<v.w<<")";
}

std::ostream & operator<<(std::ostream & out, const glm::mat3 & m)
{
    out<<"[["<<m[0].x<<","<<m[1].x<<","<<m[2].x<<"\n";
    out<<" ["<<m[0].y<<","<<m[1].y<<","<<m[2].y<<"\n";
    return out<<" ["<<m[0].z<<","<<m[1].z<<","<<m[2].z<<"]]";
}

std::ostream & operator<<(std::ostream & out, const glm::mat4 & m)
{
    out<<"[["<<m[0].x<<","<<m[1].x<<","<<m[2].x<<","<<m[3].x<<"]\n";
    out<<" ["<<m[0].y<<","<<m[1].y<<","<<m[2].y<<","<<m[3].y<<"]\n";
    out<<" ["<<m[0].z<<","<<m[1].z<<","<<m[2].z<<","<<m[3].z<<"]\n";
    return out<<" ["<<m[0].w<<","<<m[1].w<<","<<m[2].w<<","<<m[3].w<<"]]";
}

std::ostream & operator<<(std::ostream & out, const glm::quat & q)
{
    return out<<"("<<q.x<<","<<q.y<<","<<q.z<<","<<q.w<<")";
}

std::string to_string(const glm::vec2 & v)
{
    std::ostringstream str;
    str<<v;
    return str.str();
}

std::string to_string(const glm::vec3 & v)
{
    std::ostringstream str;
    str<<v;
    return str.str();
}

std::string to_string(const glm::vec4 & v)
{
    std::ostringstream str;
    str<<v;
    return str.str();
}

std::string to_string(const glm::mat3 & m)
{
    std::ostringstream str;
    str<<m;
    return str.str();
}

std::string to_string(const glm::mat4 & m)
{
    std::ostringstream str;
    str<<m;
    return str.str();
}

std::string to_string(const glm::quat & q)
{
    std::ostringstream str;
    str<<q;
    return str.str();
}
