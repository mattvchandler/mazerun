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

#include <SFML/Graphics.hpp>

#include "texture.hpp"

Texture::Texture(): _texid(0)
{
}

Texture::~Texture()
{
    if(_texid)
    {
        glDeleteTextures(1, &_texid);
    }
}

// TODO should all inits should return bool OR throw (pick one)
// create a cubemap texture from 6 filenames
void Texture_cubemap::init(const std::string & left_fname, const std::string & right_fname,
    const std::string & back_fname, const std::string & front_fname,
    const std::string & down_fname, const std::string & up_fname)
{
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

    glGenTextures(1, &_texid);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texid);

    for(const auto & filename: filenames)
    {
        // load each file
        sf::Image img;
        if(!img.loadFromFile(filename.first))
        {
            throw std::ios_base::failure(std::string("Error reading image file: ") + filename.first);
        }

        // send data to OpenGL
        glTexImage2D(filename.second, 0, GL_RGBA, img.getSize().x, img.getSize().y,
            0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
    }

    // set params
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Texture_cubemap::bind()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texid);
}
