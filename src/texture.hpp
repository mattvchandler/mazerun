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

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>

class Texture: public sf::NonCopyable
{
public:
    virtual ~Texture();
    virtual void bind() const = 0;
    GLuint get_id() const;
protected:
    Texture();

    GLuint _texid;
    std::string _key;
    static std::unordered_map<std::string, std::weak_ptr<Texture>> _allocated_tex;
};

class Texture_2D final: public Texture
{
public:
    static std::shared_ptr<Texture_2D> create(const std::string & filename);
    void bind() const override;
private:
    Texture_2D(const std::string & filename);
};

class Texture_cubemap final: public Texture
{
public:
    static std::shared_ptr<Texture_cubemap> create(const std::string & left_fname, const std::string & right_fname,
        const std::string & back_fname, const std::string & front_fname,
        const std::string & down_fname, const std::string & up_fname);
    void bind() const override;
private:
    Texture_cubemap(const std::string & left_fname, const std::string & right_fname,
        const std::string & back_fname, const std::string & front_fname,
        const std::string & down_fname, const std::string & up_fname);
};

#endif // TEXTURE_HPP
