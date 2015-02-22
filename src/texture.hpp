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
#include <unordered_map>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>

class Texture: public sf::NonCopyable
{
public:
    Texture(const std::string & filename);
    virtual ~Texture();
protected:
    GLuint _texid;
    std::string _filename;
    static std::unordered_map<std::string, std::pair<unsigned int, GLuint>> _allocated_tex;
private:
    virtual void bind() const = 0;
    GLuint operator()() const;
};

class Texture_2D: public Texture
{
public:
    Texture_2D(const std::string & filename);
    void bind() const override;
};

class Texture_cubemap: public Texture
{
public:
    Texture_cubemap(const std::string & left_fname, const std::string & right_fname,
        const std::string & back_fname, const std::string & front_fname,
        const std::string & down_fname, const std::string & up_fname);
    void bind() const override;
};

#endif // TEXTURE_HPP
