// texture.hpp
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

#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>

// TODO: have materials and FBO's contain tex ID and provide a lookup function?

class Texture: public sf::NonCopyable
{
public:
    virtual ~Texture();
    virtual void bind() const = 0;
    GLuint get_id() const;

protected:
    Texture();

    GLuint _texid;
};

class Texture_2D final: public Texture
{
public:
    Texture_2D() = default;
    void bind() const override;

    static Texture_2D * create(const std::string & filename, const GLenum internal_format = GL_RGBA);
    static Texture_2D * create_rgb_and_alpha(const std::string & rgb_filename,
        const std::string & alpha_filename);

    static Texture_2D * white_fallback();
    static Texture_2D * black_fallback();
    static Texture_2D * missing_fallback();
    static Texture_2D * normal_map_fallback();
};

class Texture_cubemap final: public Texture
{
public:
    Texture_cubemap() = default;
    void bind() const override;

    static Texture_cubemap * create(const std::string & left_fname, const std::string & right_fname,
        const std::string & back_fname, const std::string & front_fname,
        const std::string & down_fname, const std::string & up_fname,
        const GLenum internal_format = GL_RGBA);
    static Texture_cubemap * env_fallback();
};

struct Texture_cache
{
    std::unordered_map<std::string, std::unique_ptr<Texture>> tex_index;

    std::unique_ptr<Texture_2D> white_fallback;
    std::unique_ptr<Texture_2D> black_fallback;
    std::unique_ptr<Texture_2D> missing_fallback;
    std::unique_ptr<Texture_2D> normal_map_fallback;

    std::unique_ptr<Texture_cubemap> env_fallback;
};

class Texture_cache_locator
{
public:
    Texture_cache_locator() = delete;
    ~Texture_cache_locator() = delete;

    static void init(Texture_cache * cache);
    static Texture_cache & get();

private:
    static Texture_cache  _default_texture_cache;
    static Texture_cache * _cache;
};

#endif // TEXTURE_HPP
