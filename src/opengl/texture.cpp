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

#include "opengl/texture.hpp"

#include <GL/glew.h>
#include <SFML/Graphics.hpp>

#include "util/logger.hpp"

Texture::~Texture()
{
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

Texture_2D * Texture_2D::create(const std::string & filename, const GLenum internal_format)
{
    std::string key = std::string("2D:") + filename;

    if(Texture_cache_locator::get().tex_index.count(key) > 0)
    {
        Logger_locator::get()(Logger::DBG, "Reusing texture: " + key);
        return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().tex_index[key].get());
    }
    else
    {
        Logger_locator::get()(Logger::DBG, "Loading 2D texture: " + filename);

        sf::Image img;
        if(!img.loadFromFile(filename))
        {
            Logger_locator::get()(Logger::ERROR, std::string("Error reading image file: ") + filename);
            throw std::ios_base::failure(std::string("Error reading image file: ") + filename);
        }

        Texture_2D * ret = new Texture_2D;
        ret->bind();

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img.getSize().x, img.getSize().y,
            0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());

        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Texture_cache_locator::get().tex_index.emplace(key, std::unique_ptr<Texture>(ret));
        return ret;
    }
}
Texture_2D * Texture_2D::create_rgb_and_alpha(const std::string & rgb_filename,
    const std::string & alpha_filename)
{
    std::string key = std::string("2D RGB+A:") + rgb_filename + "+" + alpha_filename;

    if(Texture_cache_locator::get().tex_index.count(key) > 0)
    {
        Logger_locator::get()(Logger::DBG, "Reusing texture: " + key);
        return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().tex_index[key].get());
    }
    else
    {
        Logger_locator::get()(Logger::DBG, "Loading 2D texture: " + rgb_filename + "+" + alpha_filename);

        sf::Image rgb_img;
        if(!rgb_img.loadFromFile(rgb_filename))
        {
            Logger_locator::get()(Logger::ERROR, std::string("Error reading image file: ") + rgb_filename);
            throw std::ios_base::failure(std::string("Error reading image file: ") + rgb_filename);
        }

        sf::Image alpha_img;
        if(!alpha_img.loadFromFile(alpha_filename))
        {
            Logger_locator::get()(Logger::ERROR, std::string("Error reading image file: ") + alpha_filename);
            throw std::ios_base::failure(std::string("Error reading image file: ") + alpha_filename);
        }

        if(rgb_img.getSize() != alpha_img.getSize())
        {
            Logger_locator::get()(Logger::ERROR, std::string("Error: Image sizes do not match: ") + rgb_filename +
                "+" + alpha_filename);
            throw std::ios_base::failure(std::string("Error: Image sizes do not match: ") + rgb_filename +
                "+" + alpha_filename);
        }

        std::vector<sf::Uint8> data(rgb_img.getSize().x * rgb_img.getSize().y * 4);

        for(std::size_t i = 0; i < rgb_img.getSize().x * rgb_img.getSize().y; ++i)
        {
            data[i * 3] = rgb_img.getPixelsPtr()[i * 4];
            data[i * 3 + 1] = rgb_img.getPixelsPtr()[i * 4 + 1];
            data[i * 3 + 2] = rgb_img.getPixelsPtr()[i * 4 + 2];
            data[i * 3 + 3] = alpha_img.getPixelsPtr()[i * 4];
        }

        Texture_2D * ret = new Texture_2D;
        ret->bind();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgb_img.getSize().x, rgb_img.getSize().y,
            0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Texture_cache_locator::get().tex_index.emplace(key, std::unique_ptr<Texture>(ret));
        return ret;
    }
}

Texture_2D * Texture_2D::create_to_alpha(const std::string & filename,
    const glm::vec3 & rgb)
{
    std::string key = std::string("2D ALPHA:") + filename;

    if(Texture_cache_locator::get().tex_index.count(key) > 0)
    {
        Logger_locator::get()(Logger::DBG, "Reusing texture: " + key);
        return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().tex_index[key].get());
    }
    else
    {
        Logger_locator::get()(Logger::DBG, "Loading 2D texture: " + filename);

        sf::Image img;
        if(!img.loadFromFile(filename))
        {
            Logger_locator::get()(Logger::ERROR, std::string("Error reading image file: ") + filename);
            throw std::ios_base::failure(std::string("Error reading image file: ") + filename);
        }

        std::vector<sf::Uint8> data(img.getSize().x * img.getSize().y * 4);

        for(std::size_t i = 0; i < img.getSize().x * img.getSize().y; ++i)
        {
            data[i * 3] = (sf::Uint8)(glm::clamp(rgb.r, 0.0f, 1.0f) * 255.0f);
            data[i * 3 + 1] = (sf::Uint8)(glm::clamp(rgb.g, 0.0f, 1.0f) * 255.0f);
            data[i * 3 + 2] = (sf::Uint8)(glm::clamp(rgb.b, 0.0f, 1.0f) * 255.0f);
            data[i * 3 + 3] = img.getPixelsPtr()[i * 4];
        }

        Texture_2D * ret = new Texture_2D;
        ret->bind();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getSize().x, img.getSize().y,
            0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());

        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Texture_cache_locator::get().tex_index.emplace(key, std::unique_ptr<Texture>(ret));
        return ret;
    }
}

Texture_2D * Texture_2D::white_fallback()
{
    if(!Texture_cache_locator::get().white_fallback)
    {
        Texture_2D * fallback = new Texture_2D;
        fallback->bind();

        glm::vec3 color(1.0f, 1.0f, 1.0f);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, &color[0]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Texture_cache_locator::get().white_fallback.reset(fallback);
        Logger_locator::get()(Logger::DBG, "Generated white fallback texture");
    }
    return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().white_fallback.get());
}

Texture_2D * Texture_2D::black_fallback()
{
    if(!Texture_cache_locator::get().black_fallback)
    {
        Texture_2D * fallback = new Texture_2D;
        fallback->bind();

        glm::vec3 color(0.0f, 0.0f, 0.0f);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, &color[0]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Texture_cache_locator::get().black_fallback.reset(fallback);
        Logger_locator::get()(Logger::DBG, "Generated black fallback texture");
    }
    return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().black_fallback.get());
}

// black & magenta checkerboard (like source engine!)
Texture_2D * Texture_2D::missing_fallback()
{
    if(!Texture_cache_locator::get().missing_fallback)
    {
        const unsigned short size = 8;
        std::vector<glm::vec3> data(size * size);
        for(unsigned short row = 0; row < size; ++row)
        {
            for(unsigned short col = 0; col < size; ++col)
            {
                unsigned short i = row * size + col;
                // odd pixels black, even magenta
                if((row ^ col) & 1)
                    data[i] = glm::vec3(0.0f, 0.0f, 0.0f);
                else
                    data[i] = glm::vec3(1.0f, 0.0f, 1.0f);
            }
        }

        Texture_2D * fallback = new Texture_2D;
        fallback->bind();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, data.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Texture_cache_locator::get().missing_fallback.reset(fallback);
        Logger_locator::get()(Logger::DBG, "Generated missing fallback texture");
    }
    return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().missing_fallback.get());
}

Texture_2D * Texture_2D::normal_map_fallback()
{
    if(!Texture_cache_locator::get().normal_map_fallback)
    {
        Texture_2D * fallback = new Texture_2D;
        fallback->bind();

        glm::vec3 color(0.5f, 0.5f, 1.0f);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, &color[0]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Texture_cache_locator::get().normal_map_fallback.reset(fallback);
        Logger_locator::get()(Logger::DBG, "Generated normal map fallback texture");
    }
    return dynamic_cast<Texture_2D *>(Texture_cache_locator::get().normal_map_fallback.get());
}

void Texture_2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, _texid);
}

Texture_cubemap * Texture_cubemap::create(const std::string & left_fname, const std::string & right_fname,
    const std::string & back_fname, const std::string & front_fname,
    const std::string & down_fname, const std::string & up_fname,
    const GLenum internal_format)
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
        // create array of pairs: filename with type enum
        std::vector<std::pair<std::string, GLenum>> filenames =
        {
            {left_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
            {right_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_X},
            {back_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
            {front_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
            {down_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
            {up_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_Y}
        };

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // TODO: should this be enabled at higher scope?

        Texture_cubemap * ret = new Texture_cubemap;
        ret->bind();

        for(const auto & filename: filenames)
        {
            Logger_locator::get()(Logger::DBG, "Loading Cubemap texture: " + filename.first);
            // load each file
            sf::Image img;
            if(!img.loadFromFile(filename.first))
            {
                delete ret;
                Logger_locator::get()(Logger::ERROR, std::string("Error reading image file: ") + filename.first);
                throw std::ios_base::failure(std::string("Error reading image file: ") + filename.first);
            }

            // send data to OpenGL
            glTexImage2D(filename.second, 0, internal_format, img.getSize().x, img.getSize().y,
                0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
        }

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        Texture_cache_locator::get().tex_index.emplace(key, std::unique_ptr<Texture>(ret));
        return ret;
    }
}

Texture_cubemap * Texture_cubemap::env_fallback()
{
    if(!Texture_cache_locator::get().env_fallback)
    {
        Texture_cubemap * fallback = new Texture_cubemap;
        fallback->bind();

        std::vector<GLenum> sides =
        {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

        glm::vec3 color(1.0f, 1.0f, 1.0f);
        for(const auto & side: sides)
        {
            glTexImage2D(side, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, &color[0]);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        Texture_cache_locator::get().env_fallback.reset(fallback);
        Logger_locator::get()(Logger::DBG, "Generated cubemap fallback texture");
    }
    return dynamic_cast<Texture_cubemap *>(Texture_cache_locator::get().env_fallback.get());
}

void Texture_cubemap::bind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texid);
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
