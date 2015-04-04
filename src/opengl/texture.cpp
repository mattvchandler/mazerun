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

#include "opengl/texture.hpp"

#include <GL/glew.h>
#include <SFML/Graphics.hpp>

#include "util/logger.hpp"

Texture::~Texture()
{
    if(!_key.empty())
        Logger_locator::get()(Logger::DBG, "Unloading texture: " + _key);

    Logger_locator::get()(Logger::TRACE, "Deleting GL texture: " + std::to_string(_texid));
    glDeleteTextures(1, &_texid);
}

GLuint Texture::get_id() const
{
    return  _texid;
}

Texture::Texture()
{
    glGenTextures(1, &_texid);
    Logger_locator::get()(Logger::TRACE, "Generated GL texture: " + std::to_string(_texid));
}

Texture_2D * Texture_2D::create(const std::string & filename)
{
    std::string key = std::string("2D:") + filename;

    if(Texture_cache_locator::get().tex_index.count(key) > 0)
    {
        Logger_locator::get()(Logger::DBG, "Reusing texture: " + key);
        return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().tex_index[key].get());
    }
    else
    {
        Texture_2D * ret = new Texture_2D(filename);
        ret->_key = key;
        Texture_cache_locator::get().tex_index.emplace(key, std::unique_ptr<Texture>(ret));
        return ret;
    }
}

Texture_2D * Texture_2D::white_fallback()
{
    if(!Texture_cache_locator::get().white_fallback)
    {
        Texture_cache_locator::get().white_fallback.reset(new Texture_2D(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1, 1));
        Logger_locator::get()(Logger::DBG, "Generated white fallback texture");
    }
    return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().white_fallback.get());
}

Texture_2D * Texture_2D::black_fallback()
{
    if(!Texture_cache_locator::get().black_fallback)
    {
        Texture_cache_locator::get().black_fallback.reset(new Texture_2D(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1, 1));
        Logger_locator::get()(Logger::DBG, "Generated black fallback texture");
    }
    return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().black_fallback.get());
}

// black & magenta checkerboard (like source!)
Texture_2D * Texture_2D::missing_fallback()
{
    if(!Texture_cache_locator::get().missing_fallback)
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

        Texture_cache_locator::get().missing_fallback.reset(new Texture_2D(data, size, size));
        Logger_locator::get()(Logger::DBG, "Generated missing fallback texture");
    }
    return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().missing_fallback.get());
}

Texture_2D * Texture_2D::normal_map_fallback()
{
    if(!Texture_cache_locator::get().normal_map_fallback)
    {
        Texture_cache_locator::get().normal_map_fallback.reset(new Texture_2D(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f), 1, 1));
        Logger_locator::get()(Logger::DBG, "Generated normal map fallback texture");
    }
    return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().normal_map_fallback.get());
}

void Texture_2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, _texid);
}

Texture_2D::Texture_2D(const std::string & filename)
{
    Logger_locator::get()(Logger::DBG, "Loading 2D texture: " + filename);
    glBindTexture(GL_TEXTURE_2D, _texid);

    sf::Image img;
    if(!img.loadFromFile(filename))
    {
        Logger_locator::get()(Logger::ERROR, std::string("Error reading image file: ") + filename);
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

Texture_cubemap * Texture_cubemap::create(const std::string & left_fname, const std::string & right_fname,
    const std::string & back_fname, const std::string & front_fname,
    const std::string & down_fname, const std::string & up_fname)
{
    std::string key = std::string("CUBE:") +
        left_fname + ";" + right_fname + ";" +
        back_fname + ";" + front_fname + ";" +
        down_fname + ";" + up_fname;

    if(Texture_cache_locator::get().tex_index.count(key) > 0)
    {
        Logger_locator::get()(Logger::DBG, "Reusing texture: " + key);
        return dynamic_cast<Texture_cubemap *>(Texture_cache_locator::get().tex_index[key].get());
    }
    else
    {
        Texture_cubemap * ret(new Texture_cubemap(left_fname, right_fname,
                back_fname, front_fname,
                down_fname, up_fname));
        ret->_key = key;
        Texture_cache_locator::get().tex_index.emplace(key, std::unique_ptr<Texture>(ret));
        return ret;
    }
}

Texture_cubemap * Texture_cubemap::env_fallback()
{
    if(!Texture_cache_locator::get().env_fallback)
    {
        Texture_cache_locator::get().env_fallback.reset(new Texture_cubemap(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1, 1));
        Logger_locator::get()(Logger::DBG, "Generated cubemap fallback texture");
    }
    return dynamic_cast<Texture_cubemap *>(Texture_cache_locator::get().env_fallback.get());
}

void Texture_cubemap::bind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texid);
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
        Logger_locator::get()(Logger::DBG, "Loading Cubemap texture: " + filename.first);
        // load each file
        sf::Image img;
        if(!img.loadFromFile(filename.first))
        {
            Logger_locator::get()(Logger::ERROR, std::string("Error reading image file: ") + filename.first);
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

Texture_cache Texture_cache_locator::_default_texture_cache;
Texture_cache * Texture_cache_locator::_cache = &Texture_cache_locator::_default_texture_cache;

void Texture_cache_locator::init(Texture_cache * cache)
{
    if(!cache)
    {
        _default_texture_cache.tex_index.clear();

        _default_texture_cache.white_fallback.reset();
        _default_texture_cache.black_fallback.reset();
        _default_texture_cache.missing_fallback.reset();
        _default_texture_cache.normal_map_fallback.reset();
        _default_texture_cache.env_fallback.reset();

        _cache = &_default_texture_cache;
    }
    else
        _cache = cache;
}

Texture_cache & Texture_cache_locator::get()
{
    return *_cache;
}
