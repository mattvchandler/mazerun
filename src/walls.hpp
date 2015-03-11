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

#include "grid.hpp"
#include "model.hpp"

class Walls final: public Model
{
public:
    static std::shared_ptr<Walls> create(const unsigned int width, const unsigned int height);
    void draw(const std::function<void(const Material &)> & set_material) const;

private:
    Walls(const unsigned int width, const unsigned int height);

    Grid _grid;
};

Entity create_walls(const unsigned int width, const unsigned int height);

class Floor final: public Model
{
public:
static std::shared_ptr<Floor> create(const unsigned int width, const unsigned int height);
    void draw(const std::function<void(const Material &)> & set_material) const;

private:
    Floor(const unsigned int width, const unsigned int height);
};

Entity create_floor(const unsigned int width, const unsigned int height);

#endif // WALLS_HPP
