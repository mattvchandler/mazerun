// gen_rooms.cpp
// room generation

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

#include "mazegen/grid.hpp"

#include <algorithm>
#include <functional>
#include <random>

#include "mazegen/disjoint_set.hpp"

extern thread_local std::mt19937 prng;

bool attempt_gen_room(const std::vector<std::vector<Grid_cell>> & grid,
    sf::Vector2u & pos_out, sf::Vector2u & size_out)
{
    size_out = sf::Vector2u(std::binomial_distribution<unsigned int>(std::min(9u, (unsigned int)(grid[0].size() - 1)), 0.5)(prng) + 1,
        std::binomial_distribution<unsigned int>(std::min(9u, (unsigned int)(grid.size() - 1)), 0.5)(prng) + 1);

    // discard skinny rooms
    if((float)size_out.x / (float)size_out.y > 4 || (float)size_out.y / (float)size_out.x > 4)
        return false;

    pos_out = sf::Vector2u(std::uniform_int_distribution<unsigned int>(0, std::max(0, (int)grid[0].size() - (int)size_out.x - 1))(prng),
        std::uniform_int_distribution<unsigned int>(0, std::max(0, (int)grid.size() - (int)size_out.y - 1))(prng));

    // check if overlapping
    for(std::size_t row = pos_out.y; row < pos_out.y + size_out.y; ++row)
    {
        for(std::size_t col = pos_out.x; col < pos_out.x + size_out.x; ++col)
        {
            if(grid[row][col].visited)
            {
                return false;
            }
        }
    }

    return true;
}

void place_room(std::vector<std::vector<Grid_cell>> & grid,
    const sf::Vector2u & pos, const sf::Vector2u & size, const int region)
{
    // mark visited
    // destroy walls, excepting room borders
    for(std::size_t row = pos.y; row < pos.y + size.y; ++row)
    {
        for(std::size_t col = pos.x; col < pos.x + size.x; ++col)
        {
            grid[row][col].visited = true;
            grid[row][col].region = region;
            grid[row][col].room = true;

            if(row > pos.y)
                grid[row][col].walls[UP] = false;
            if(row < pos.y + size.y - 1)
                grid[row][col].walls[DOWN] = false;
            if(col > pos.x)
                grid[row][col].walls[LEFT] = false;
            if(col < pos.x + size.x - 1)
                grid[row][col].walls[RIGHT] = false;
        }
    }
}

std::vector<Wall> find_connectors(const std::vector<std::vector<Grid_cell>> & grid)
{
    std::vector<Wall> connectors;
    for(std::size_t row = 0; row < grid.size() - 1; ++row)
    {
        for(std::size_t col = 0; col < grid[0].size() - 1; ++col)
        {
            if(grid[row][col].region != grid[row][col + 1].region)
            {
                sf::Vector2u cell_1(col, row);
                sf::Vector2u cell_2(col + 1, row);
                int region_1 = grid[row][col].region;
                int region_2 = grid[row][col + 1].region;

                connectors.push_back({cell_1, cell_2, RIGHT, LEFT, region_1, region_2});
            }
            if(grid[row][col].region != grid[row + 1][col].region)
            {
                sf::Vector2u cell_1(col, row);
                sf::Vector2u cell_2(col, row + 1);
                int region_1 = grid[row][col].region;
                int region_2 = grid[row + 1][col].region;

                connectors.push_back({cell_1, cell_2, DOWN, UP, region_1, region_2});
            }
        }
    }
    return connectors;
}

void join_regions(std::vector<std::vector<Grid_cell>> & grid,
    const unsigned int num_regions)
{
    // find connectors (walls that separate 2 different regions
    std::vector<Wall> connectors = find_connectors(grid);

    // shuffle connectors
    std::shuffle(connectors.begin(), connectors.end(), prng);

    // use Kruskal's alg to find min spanning tree of region graph
    std::vector<int> regions_vec(num_regions);
    for(std::size_t i = 0; i < regions_vec.size(); ++i)
        regions_vec[i] = i;

    Disjoint_set<int>regions(regions_vec);

    int num_sets = num_regions;
    for(const auto & conn: connectors)
    {
        int set_1 = regions.find_rep(conn.region_1);
        int set_2 = regions.find_rep(conn.region_2);

        if(set_1 != set_2)
        {
            regions.union_reps(set_1, set_2);

            // destroy walls joining regions
            grid[conn.cell_1.y][conn.cell_1.x].walls[conn.dir_1] = false;
            grid[conn.cell_2.y][conn.cell_2.x].walls[conn.dir_2] = false;

            if(--num_sets == 1)
                break;
        }
    }
}

void destroy_rand_walls(std::vector<std::vector<Grid_cell>> & grid,
    const unsigned int wall_rm_attempts)
{
    //  randomly destroy random walls
    for(unsigned int i = 0; i  < wall_rm_attempts; ++i)
    {
        sf::Vector2u cell(std::uniform_int_distribution<unsigned int>(0, grid[0].size() - 1)(prng),
            std::uniform_int_distribution<unsigned int>(0, grid.size() - 1)(prng));

        Direction wall = (Direction)std::uniform_int_distribution<int>(0, 3)(prng);

        switch(wall)
        {
        case UP:
            if(cell.y > 0)
            {
                grid[cell.y][cell.x].walls[UP] = false;
                grid[cell.y - 1][cell.x].walls[DOWN] = false;
            }
            break;
        case DOWN:
            if(cell.y < grid.size() - 1)
            {
                grid[cell.y][cell.x].walls[DOWN] = false;
                grid[cell.y + 1][cell.x].walls[UP] = false;
            }
            break;
        case LEFT:
            if(cell.x > 0)
            {
                grid[cell.y][cell.x].walls[LEFT] = false;
                grid[cell.y][cell.x - 1].walls[RIGHT] = false;
            }
            break;
        case RIGHT:
            if(cell.x < grid[0].size() - 1)
            {
                grid[cell.y][cell.x].walls[RIGHT] = false;
                grid[cell.y][cell.x + 1].walls[LEFT] = false;
            }
            break;
        }
    }
}

void Grid::gen_rooms(const Mazegen_alg mazegen,
    const unsigned int room_attempts, const unsigned int wall_rm_attempts)
{
    int region = 0;
    // place some random rooms
    for(unsigned int i = 0; i  < room_attempts; ++i)
    {
        sf::Vector2u pos, size;
        if(!attempt_gen_room(grid, pos, size))
            continue;

        place_room(grid, pos, size, region++);
    }

    // get mazegen function
    std::function<void(Grid &, const sf::Vector2u &, const int)> mazegen_f;
    switch(mazegen)
    {
    case MAZEGEN_DFS:
        mazegen_f = &Grid::mazegen_dfs;
        break;
    case MAZEGEN_PRIM:
        mazegen_f = &Grid::mazegen_kruskal;
        break;
    case MAZEGEN_KRUSKAL:
        mazegen_f = &Grid::mazegen_kruskal;
        break;
    }

    // fill in remaining areas with mazes
    for(std::size_t row = 0; row < grid.size(); ++row)
    {
        for(std::size_t col = 0; col < grid[0].size(); ++col)
        {
            if(!grid[row][col].visited)
            {
                mazegen_f(*this, sf::Vector2u(col, row), region++);
            }
        }
    }

    join_regions(grid, region);

    // destroy some walls to create multiple paths between cells
    destroy_rand_walls(grid, wall_rm_attempts);
}
