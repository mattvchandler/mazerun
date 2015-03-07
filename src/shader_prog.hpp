// shader.hpp
// shader program creation & managment

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

#ifndef SHADER_PROG_HPP
#define SHADER_PROG_HPP

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>

class Shader_prog final: public sf::NonCopyable
{
public:
    Shader_prog(const std::vector<std::pair<std::string, GLenum>> & sources,
        const std::vector<std::pair<std::string, GLuint>> & attribs);
    ~Shader_prog();
    void add_uniform(const std::string & uniform);
    GLuint get_uniform(const std::string & uniform) const;
    void use() const;
    GLuint get_id() const;
    // TODO: probably my own exceptions, rather than use system exceptions

protected:
    std::unordered_map<std::string, GLuint> _uniforms; // convenience storage for uniform values
    GLuint _prog;
};

#endif // SHADER_PROG_HPP
