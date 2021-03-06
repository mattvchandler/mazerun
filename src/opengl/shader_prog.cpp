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

#include "opengl/shader_prog.hpp"

#include <fstream>
#include <stdexcept>
#include <system_error>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>

#include "util/logger.hpp"

Shader_prog::Shader_prog(const std::vector<std::pair<std::string, GLenum>> & sources,
    const std::vector<std::pair<std::string, GLuint>> & attribs,
    const std::vector<std::pair<std::string, GLuint>> & frag_data)
{
    std::vector<Shader_obj *> shaders;
    for(const auto & source: sources)
    {
        auto shader_it = _shader_cache.find(source.first);
        if(shader_it == _shader_cache.end())
        {
            shader_it = _shader_cache.emplace(source.first, std::unique_ptr<Shader_obj>(new Shader_obj(source.first, source.second))).first;
        }
        shaders.push_back(shader_it->second.get());
    }

    _prog = glCreateProgram();
    Logger_locator::get()(Logger::TRACE, "Generating shader prog id: " + std::to_string(_prog));

    for(auto & shader: shaders)
        glAttachShader(_prog, shader->get_id());

    // bind given attributes (must be done before link)
    for(auto & attr: attribs)
        glBindAttribLocation(_prog, attr.second, attr.first.c_str());

    for(auto & frag_out: frag_data)
        glBindFragDataLocation(_prog, frag_out.second, frag_out.first.c_str());

    Logger_locator::get()(Logger::DBG, "Linking shaders");
    glLinkProgram(_prog);

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

        Logger_locator::get()(Logger::ERROR, std::string("Error linking shader program:\n") + std::string(log.data()));
        throw std::system_error(link_status, std::system_category(), std::string("Error linking shader program:\n") +
            std::string(log.data()));
    }

    // get uniforms
    GLint num_uniforms;
    GLint max_buff_size;
    glGetProgramiv(_prog, GL_ACTIVE_UNIFORMS, &num_uniforms);
    glGetProgramiv(_prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_buff_size);

    std::vector<GLchar> uniform_buff(max_buff_size);

    for(GLint i = 0; i < num_uniforms; ++i)
    {
        // ignored, but required
        GLsizei buff_size;

        glGetActiveUniformName(_prog, i, uniform_buff.size(), &buff_size, uniform_buff.data());
        std::string uniform = uniform_buff.data();

        GLint loc = glGetUniformLocation(_prog, uniform.c_str());
        if(loc != -1)
        {
            _uniforms[uniform] = loc;
            Logger_locator::get()(Logger::TRACE, "Found uniform: " + uniform + " at " + std::to_string(loc));
        }
    }

    glUseProgram(0); // TODO: get prev val
}

Shader_prog::~Shader_prog()
{
    Logger_locator::get()(Logger::TRACE, "Deleting shader prog id: " + std::to_string(_prog));
    glDeleteProgram(_prog);
}

GLint Shader_prog::get_uniform(const std::string & uniform) const
{
    try
    {
        return _uniforms.at(uniform);
    }
    catch(std::out_of_range & e)
    {
        Logger_locator::get()(Logger::WARN, "Unknown uniform: " + uniform);
        throw std::out_of_range("Unknown uniform: " + uniform);
    }
}

void Shader_prog::use() const
{
    glUseProgram(_prog);
}

GLuint Shader_prog::get_id() const
{
    return _prog;
}

void Shader_prog::clear_cache()
{
    _shader_cache.clear();
}

Shader_prog::Shader_obj::Shader_obj(const std::string & filename, const GLenum shader_type)
{
    Logger_locator::get()(Logger::DBG, "Loading shader: " + filename);

    // open shader file
    std::ifstream in(filename, std::ios::binary | std::ios::in);
    std::vector <char> buff;

    if(in)
    {
        in.seekg(0, std::ios::end);
        std::size_t in_size = in.tellg();
        in.seekg(0, std::ios::beg);

        buff.resize(in_size + 1);
        buff.back() = '\0';
        in.read(buff.data(), in_size);

        if(!in)
        {
            Logger_locator::get()(Logger::ERROR, std::string("Error reading shader file: ") + filename);
            throw std::ios_base::failure(std::string("Error reading shader file: ") + filename);
        }
    }
    else
    {
        Logger_locator::get()(Logger::ERROR, std::string("Error opening shader file: ") + filename);
        throw std::ios_base::failure(std::string("Error opening shader file: ") + filename);
    }

    const char * source_str = buff.data();

    _id = glCreateShader(shader_type);
    Logger_locator::get()(Logger::TRACE, "Generating shader id: " + std::to_string(_id));

    // compile shaders
    glShaderSource(_id, 1, &source_str, NULL);
    glCompileShader(_id);

    // check for compilation errors
    GLint compile_status;
    glGetShaderiv(_id, GL_COMPILE_STATUS, &compile_status);

    if(compile_status != GL_TRUE)
    {
        GLint log_length;
        glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetShaderInfoLog(_id, log_length, NULL, log.data());

        glDeleteShader(_id);

        Logger_locator::get()(Logger::ERROR, std::string("Error compiling shader: ") + filename + std::string("\n") + std::string(log.data()));
        throw std::system_error(compile_status, std::system_category(), std::string("Error compiling shader: ") +
            filename + std::string("\n") + std::string(log.data()));
    }
}

Shader_prog::Shader_obj::~Shader_obj()
{
    Logger_locator::get()(Logger::DBG, "Deleting shader id: " + std::to_string(_id));
    glDeleteShader(_id);
}

GLuint Shader_prog::Shader_obj::get_id() const
{
    return _id;
}

std::unordered_map<std::string, std::unique_ptr<Shader_prog::Shader_obj>> Shader_prog::_shader_cache;
