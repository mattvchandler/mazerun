// grid.cpp
// maze grid representation and algs

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
#include <stdexcept>

#include "grid.hpp"

Grid_cell::Grid_cell(): visited(false), region(-1), room(false)
{
    for(int i = 0; i < 4; ++i)
        walls[i] = true;
}

Grid::Grid(const sf::Vector2u & grid_size)
    :grid(grid_size.y, std::vector<Grid_cell>(grid_size.x))
{
    if(grid_size.y == 0)
    {
        throw std::invalid_argument("grid_size.y == 0");
    }
    if(grid_size.x == 0)
    {
        throw std::invalid_argument("grid_size.x == 0");
    }
}

Grid::Grid(const unsigned int width, const unsigned int height)
    :grid(height, std::vector<Grid_cell>(width))
{
    if(height == 0)
    {
        throw std::invalid_argument("grid_size.y == 0");
    }
    if(width == 0)
    {
        throw std::invalid_argument("grid_size.x == 0");
    }
}

void Grid::init(const Mazegen_alg mazegen,
    const unsigned int room_attempts, const unsigned int wall_rm_attempts)
{
    for(auto & row: grid)
    {
        for(auto & cell: row)
        {
            for(int i = 0; i < 4; ++i)
                cell.walls[i] = true;
        }
    }

    gen_rooms(mazegen, room_attempts, wall_rm_attempts); // TODO: parameterize
}
