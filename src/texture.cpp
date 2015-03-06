// texture.cpp
// OpenGL texture classes

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

// TODO: replace glm:: data to openGL with glm::value_ptr?

#include "texture.hpp"

#include <iostream> // TODO: remove

#include <GL/glew.h>
#include <SFML/Graphics.hpp>

std::unordered_map<std::string, std::weak_ptr<Texture>> Texture::_allocated_tex;

Texture::~Texture()
{
    glDeleteTextures(1, &_texid);
    _allocated_tex.erase(_key);
}

GLuint Texture::get_id() const
{
    return  _texid;
}

Texture::Texture()
{
    glGenTextures(1, &_texid);
}

std::shared_ptr<Texture_2D> Texture_2D::create(const std::string & filename)
{
    std::string key = std::string("2D:") + filename;

    if(Texture::_allocated_tex.count(key) > 0)
    {
        return std::dynamic_pointer_cast<Texture_2D>(Texture::_allocated_tex[key].lock());
    }
    else
    {
        std::shared_ptr<Texture_2D> ret(new Texture_2D(filename));
        ret->_key = key;
        Texture::_allocated_tex[key] = ret;
        return ret;
    }
}

Texture_2D::Texture_2D(const std::string & filename)
{
    std::cout<<"Loading texture: "<<filename<<std::endl;
    glBindTexture(GL_TEXTURE_2D, _texid);

    sf::Image img;
    if(!img.loadFromFile(filename))
    {
        throw std::ios_base::failure(std::string("Error reading image file: ") + filename);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.getSize().x, img.getSize().y,
        0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());

    set_properties();
}

Texture_2D::Texture_2D(const std::vector<glm::vec4> & data, const GLint width, const GLint height)
{
    glBindTexture(GL_TEXTURE_2D, _texid);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
        0, GL_RGBA, GL_FLOAT, &data.data()[0]);

    set_properties();
}

Texture_2D::Texture_2D(const glm::vec4 & color, const GLint width, const GLint height)
{
    glBindTexture(GL_TEXTURE_2D, _texid);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
        0, GL_RGBA, GL_FLOAT, &color[0]);

    set_properties();
}

void Texture_2D::set_properties() const
{
    glGenerateMipmap(GL_TEXTURE_2D);

    // set params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

std::shared_ptr<Texture_2D> Texture_2D::white_fallback()
{
    static std::shared_ptr<Texture_2D> fallback;
    if(!fallback)
    {
        fallback.reset(new Texture_2D(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1, 1));
    }
    return fallback;
}

std::shared_ptr<Texture_2D> Texture_2D::black_fallback()
{
    static std::shared_ptr<Texture_2D> fallback;
    if(!fallback)
    {
        fallback.reset(new Texture_2D(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1, 1));
    }
    return fallback;
}

// black & magenta checkerboard (like source!)
std::shared_ptr<Texture_2D> Texture_2D::diffuse_map_fallback()
{
    static std::shared_ptr<Texture_2D> fallback;
    if(!fallback)
    {
        const unsigned short size = 8;
        std::vector<glm::vec4> data(size * size);
        for(unsigned short row = 0; row < size; ++row)
        {
            for(unsigned short col = 0; col < size; ++col)
            {
                unsigned short i = row * size + col;
                // odd pixels black, even magenta
                if((row ^ col) & 1)
                    data[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                else
                    data[i] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
            }
        }

        fallback.reset(new Texture_2D(data, size, size));
    }
    return fallback;
}

std::shared_ptr<Texture_2D> Texture_2D::normal_map_fallback()
{
    static std::shared_ptr<Texture_2D> fallback;
    if(!fallback)
    {
        fallback.reset(new Texture_2D(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f), 1, 1));
    }
    return fallback;
}

void Texture_2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, _texid);
}

std::shared_ptr<Texture_cubemap> Texture_cubemap::create(const std::string & left_fname, const std::string & right_fname,
    const std::string & back_fname, const std::string & front_fname,
    const std::string & down_fname, const std::string & up_fname)
{
    std::string key = std::string("CUBE:") +
        left_fname + ";" + right_fname + ";" +
        back_fname + ";" + front_fname + ";" +
        down_fname + ";" + up_fname;

    if(Texture::_allocated_tex.count(key) > 0)
    {
        return std::dynamic_pointer_cast<Texture_cubemap>(Texture::_allocated_tex[key].lock());
    }
    else
    {
        std::shared_ptr<Texture_cubemap> ret(new Texture_cubemap(left_fname, right_fname,
                back_fname, front_fname,
                down_fname, up_fname));
        ret->_key = key;
        Texture::_allocated_tex[key] = ret;
        return ret;
    }
}

// create a cubemap texture from 6 filenames
Texture_cubemap::Texture_cubemap(const std::string & left_fname, const std::string & right_fname,
    const std::string & back_fname, const std::string & front_fname,
    const std::string & down_fname, const std::string & up_fname)
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // TODO: should this be enabled at higher scope?

    // create array of pairs: filename with type enum
    std::vector<std::pair<std::string, GLenum>> filenames =
    {
        std::make_pair(left_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_X),
        std::make_pair(right_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_X),
        std::make_pair(back_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z),
        std::make_pair(front_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_Z),
        std::make_pair(down_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y),
        std::make_pair(up_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
    };

    glBindTexture(GL_TEXTURE_CUBE_MAP, _texid);

    for(const auto & filename: filenames)
    {
        std::cout<<"Loading texture: "<<filename.first<<std::endl;
        // load each file
        sf::Image img;
        if(!img.loadFromFile(filename.first))
        {
            throw std::ios_base::failure(std::string("Error reading image file: ") + filename.first);
        }

        // send data to OpenGL
        glTexImage2D(filename.second, 0, GL_RGBA8, img.getSize().x, img.getSize().y,
            0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
    }

    set_properties();
}

Texture_cubemap::Texture_cubemap(const glm::vec4 & color, const GLint width, const GLint height)
{
    std::vector<GLenum> sides =
    {
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    };

    for(const auto & side: sides)
    {
        glTexImage2D(side, 0, GL_RGBA8, width, height,
            0, GL_RGBA, GL_FLOAT, &color[0]);
    }

    set_properties();
}

void Texture_cubemap::set_properties() const
{
    // set params
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

std::shared_ptr<Texture_cubemap> Texture_cubemap::env_fallback()
{
    static std::shared_ptr<Texture_cubemap> fallback;
    if(!fallback)
    {
        fallback.reset(new Texture_cubemap(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1, 1));
    }
    return fallback;
}

void Texture_cubemap::bind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texid);
}
