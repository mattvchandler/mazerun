// skybox.hpp
// Skybox info

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

#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <SFML/OpenGL.hpp>

#include "entity.hpp"
#include "renderable.hpp"
#include "shader_prog.hpp"
#include "texture.hpp"

class Skybox: public Renderable
{
public:
    Skybox();
    ~Skybox();
    void init();
    void draw(const Entity & cam, const glm::mat4 & proj) override;
protected:
    GLuint _ebo;
    Texture_cubemap _tex; // TODO: may want to expose this for environment mapping
    GLuint _num_indexes;
};

#endif // SKYBOX_HPP
