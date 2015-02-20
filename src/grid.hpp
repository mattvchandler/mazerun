// grid.hpp
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

#ifndef GRID_HPP
#define GRID_HPP

#include <SFML/System.hpp>

enum Direction {UP = 0, DOWN, LEFT, RIGHT};

class Grid_cell
{
public:
    Grid_cell();
    bool walls[4];
    bool visited;
    int region;
    bool room;
};

struct Wall
{
    sf::Vector2u cell_1, cell_2;
    Direction dir_1, dir_2;
    int region_1, region_2;
};

class Grid
{
public:
    typedef enum {MAZEGEN_DFS, MAZEGEN_PRIM, MAZEGEN_KRUSKAL} Mazegen_alg;

    Grid(const unsigned int width, const unsigned int height,
        const Mazegen_alg mazegen,
        const unsigned int room_attempts, const unsigned int wall_rm_attempts);

    std::vector<std::vector<Grid_cell>> grid;

private:
    void gen_rooms(const Mazegen_alg mazegen,
        const unsigned int room_attempts, const unsigned int wall_rm_attempts);
    void mazegen_dfs(const sf::Vector2u & start, const int region);
    void mazegen_prim(const sf::Vector2u & start, const int region);
    void mazegen_kruskal(const sf::Vector2u & start, const int region);
};

#endif // GRID_HPP
