// shader_prog.cpp
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

#include <fstream>
#include <stdexcept>
#include <system_error>

#include <GL/glew.h>

#include <SFML/Graphics.hpp>

#include "shader_prog.hpp"

Shader_prog::Shader_prog(): _prog(0)
{
}

Shader_prog::~Shader_prog()
{
    if(_prog)
        glDeleteProgram(_prog);
}

void Shader_prog::init(const std::vector<std::pair<std::string, GLenum>> & sources,
    const std::vector<std::pair<std::string, GLuint>> & attribs)
{
    std::vector<GLuint> shaders;
    for(const auto & source: sources)
    {
        // open shader file
        std::ifstream in(source.first, std::ios::binary | std::ios::in);
        std::vector <char> buff;

        if(in)
        {
            in.seekg(0, std::ios::end);
            size_t in_size = in.tellg();
            in.seekg(0, std::ios::beg);

            buff.resize(in_size + 1);
            buff.back() = '\0';
            in.read(buff.data(), in_size);

            if(!in)
            {
                for(auto & shader: shaders)
                    glDeleteShader(shader);
                throw std::ios_base::failure(std::string("Error reading shader file: ") + source.first);
            }
        }
        else
        {
            for(auto & shader: shaders)
                glDeleteShader(shader);
            throw std::ios_base::failure(std::string("Error opening shader file: ") + source.first);
        }

        // compile shaders
        shaders.push_back(glCreateShader(source.second));
        const char * source_str = buff.data();
        glShaderSource(shaders.back(), 1, &source_str, NULL);
        glCompileShader(shaders.back());

        // check for compilation errors
        GLint compile_status;
        glGetShaderiv(shaders.back(), GL_COMPILE_STATUS, &compile_status);

        if(compile_status != GL_TRUE)
        {
            GLint log_length;
            glGetShaderiv(shaders.back(), GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> log(log_length + 1);
            log.back() = '\0';
            glGetShaderInfoLog(shaders.back(), log_length, NULL, log.data());

            for(auto & shader: shaders)
                glDeleteShader(shader);

            throw std::system_error(compile_status, std::system_category(), std::string("Error compiling shader: ") +
                source.first + std::string("\n") + std::string(log.data()));
        }
    }

    _prog = glCreateProgram();

    for(auto & shader: shaders)
        glAttachShader(_prog, shader);

    // bind given attributes (must be done before link)
    for(auto & attr: attribs)
        glBindAttribLocation(_prog, attr.second, attr.first.c_str());

    glLinkProgram(_prog);

    for(auto & shader: shaders)
        glDeleteShader(shader);

    // check for link errors
    GLint link_status;
    glGetProgramiv(_prog, GL_LINK_STATUS, &link_status);
    if(link_status != GL_TRUE)
    {
        GLint log_length;
        glGetProgramiv(_prog, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetProgramInfoLog(_prog, log_length, NULL, log.data());

        glDeleteProgram(_prog);
        _prog = 0;

        throw std::system_error(link_status, std::system_category(), std::string("Error linking shader program:\n") +
            std::string(log.data()));
    }
}

bool Shader_prog::add_uniform(const std::string & uniform)
{
    GLint loc = glGetUniformLocation(_prog, uniform.c_str());
    uniforms[uniform] = loc;
    return loc != -1;
}

GLuint Shader_prog::operator()() const
{
    if(_prog)
        return _prog;
    else
        throw std::runtime_error(std::string("Attempt to use uninitialized shader prog"));
}
