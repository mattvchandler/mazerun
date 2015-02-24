// walls.hpp
// maze walls

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

#ifndef WALLS_HPP
#define WALLS_HPP

#include "gl_wrappers.hpp"
#include "grid.hpp"
#include "material.hpp"

// TODO, be a type of entity?
class Walls
{
public:
    Walls(const unsigned int width, const unsigned int height);
    void draw() const;
    const Material & get_material() const;
private:
    Grid _grid;
    GL_vertex_array _vao;
    GL_buffer _vbo;
    GLsizei _num_verts;
    Material _mat;
};

class Floor
{
public:
    Floor(const unsigned int width, const unsigned int height);
    void draw() const;
    const Material & get_material() const;
private:
    GL_vertex_array _vao;
    GL_buffer _vbo;
    GLsizei _num_verts;
    Material _mat;
};

#endif // WALLS_HPP
