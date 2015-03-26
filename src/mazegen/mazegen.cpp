// mazegen.cpp
// maze generation

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

#include <random>

#include "mazegen/disjoint_set.hpp"

extern thread_local std::mt19937 prng;

// make sf::Vector2u hashable
namespace std
{
    template <>
    struct hash<sf::Vector2u>
    {
    public:
        size_t operator()(const sf::Vector2u & a) const
        {
            return hash<unsigned int>()(a.x) ^ hash<unsigned int>()(a.y);
        }
    };
};

void Grid::mazegen_dfs(const sf::Vector2u & start, const int region)
{
    if(grid[start.y][start.x].visited)
        return;

    // to avoid stack overflow, maintain our own heap-allocated stack instead of
    //  recursing
    struct State
    {
        sf::Vector2u pos;
        Direction dirs[4];
        unsigned short dir_i;
    };
    std::vector<State> state_stack;

    state_stack.push_back({start, {UP, DOWN, LEFT, RIGHT}, 0});
    std::shuffle(std::begin(state_stack.back().dirs), std::end(state_stack.back().dirs), prng);

    grid[start.y][start.x].visited = true;
    grid[start.y][start.x].region = region;

    while(state_stack.size() > 0)
    {
        State & state = state_stack.back();
        if(state.dir_i >= 4)
        {
            state_stack.pop_back();
            continue;
        }

        sf::Vector2u next;
        bool found_next = false;

        switch(state.dirs[state.dir_i++])
        {
        case UP:
            if(state.pos.y > 0 && !grid[state.pos.y - 1][state.pos.x].visited)
            {
                next = sf::Vector2u(state.pos.x, state.pos.y - 1);
                grid[state.pos.y][state.pos.x].walls[UP] = false;
                grid[next.y][next.x].walls[DOWN] = false;
                found_next = true;
            }
            break;
        case DOWN:
            if(state.pos.y < grid.size() - 1 && !grid[state.pos.y + 1][state.pos.x].visited)
            {
                next = sf::Vector2u(state.pos.x, state.pos.y + 1);
                grid[state.pos.y][state.pos.x].walls[DOWN] = false;
                grid[next.y][next.x].walls[UP] = false;
                found_next = true;
            }
            break;
        case LEFT:
            if(state.pos.x > 0 && !grid[state.pos.y][state.pos.x - 1].visited)
            {
                next = sf::Vector2u(state.pos.x - 1, state.pos.y);
                grid[state.pos.y][state.pos.x].walls[LEFT] = false;
                grid[next.y][next.x].walls[RIGHT] = false;
                found_next = true;
            }
            break;
        case RIGHT:
            if(state.pos.x < grid[state.pos.y].size() - 1 && !grid[state.pos.y][state.pos.x + 1].visited)
            {
                next = sf::Vector2u(state.pos.x + 1, state.pos.y);
                grid[state.pos.y][state.pos.x].walls[RIGHT] = false;
                grid[next.y][next.x].walls[LEFT] = false;
                found_next = true;
            }
            break;
        }

        if(found_next)
        {
            grid[next.y][next.x].visited = true;
            grid[next.y][next.x].region = region;
            state_stack.push_back({next, {UP, DOWN, LEFT, RIGHT}, 0});
            std::shuffle(std::begin(state_stack.back().dirs), std::end(state_stack.back().dirs), prng);
        }
    }
}

void Grid::mazegen_prim(const sf::Vector2u & start, const int region)
{
    if(grid[start.y][start.x].visited)
        return;

    std::vector<Wall> walls;

    auto add_cell = [& walls, this](const sf::Vector2u & cell)
    {
        for(int i = 0; i < 4; ++i)
        {
            switch(i)
            {
            case UP:
                if(cell.y > 0 && !grid[cell.y - 1][cell.x].visited)
                {
                    walls.push_back({cell, sf::Vector2u(cell.x, cell.y - 1), UP, DOWN});
                }
                break;
            case DOWN:
                if(cell.y < grid.size() - 1 && !grid[cell.y + 1][cell.x].visited)
                {
                    walls.push_back({cell, sf::Vector2u(cell.x, cell.y + 1), DOWN, UP});
                }
                break;
            case LEFT:
                if(cell.x > 0 && !grid[cell.y][cell.x - 1].visited)
                {
                    walls.push_back({cell, sf::Vector2u(cell.x - 1, cell.y), LEFT, RIGHT});
                }
                break;
            case RIGHT:
                if(cell.x < grid[cell.y].size() - 1 && !grid[cell.y][cell.x + 1].visited)
                {
                    walls.push_back({cell, sf::Vector2u(cell.x + 1, cell.y), RIGHT, LEFT});
                }
                break;
            }
        }
    };

    add_cell(start);

    grid[start.y][start.x].visited = true;
    grid[start.y][start.x].region = region;

    while(walls.size() > 0)
    {
        std::uniform_int_distribution<std::size_t> rand_wall(0, walls.size() - 1);
        std::size_t curr_ind = rand_wall(prng);
        auto curr_it = walls.begin() + curr_ind;

        sf::Vector2u curr = curr_it->cell_1;
        Direction curr_dir = curr_it->dir_1;

        sf::Vector2u next = curr_it->cell_2;
        Direction next_dir = curr_it->dir_2;

        walls.erase(curr_it);

        if(!grid[next.y][next.x].visited)
        {
            grid[next.y][next.x].visited = true;
            grid[next.y][next.x].region = region;

            grid[curr.y][curr.x].walls[curr_dir] = false;
            grid[next.y][next.x].walls[next_dir] = false;

            add_cell(next);
        }
    }
}

void Grid::mazegen_kruskal(const sf::Vector2u & start, const int region)
{
    if(grid[start.y][start.x].visited)
        return;

    // floodfill to get all unvisited cells reachable, add walls & cells to lists, mark cell visited
    std::vector<Wall> walls;
    std::vector<sf::Vector2u> cells;

    // stack of cells to add to lists
    std::vector<sf::Vector2u> to_add = {start};

    while(to_add.size() > 0)
    {
        sf::Vector2u curr = to_add.back();
        to_add.pop_back();

        if(grid[curr.y][curr.x].visited)
            continue;

        // mark cell, add to list
        grid[curr.y][curr.x].visited = true;
        grid[curr.y][curr.x].region = region;
        cells.push_back(curr);

        // add new walls
        for(int i = 0; i < 4; ++i)
        {
            switch(i)
            {
            case UP:
                if(curr.y > 0 && !grid[curr.y - 1][curr.x].visited)
                {
                    sf::Vector2u next(curr.x, curr.y - 1);
                    walls.push_back({curr, next, UP, DOWN});
                    to_add.push_back(next);
                }
                break;
            case DOWN:
                if(curr.y < grid.size() - 1 && !grid[curr.y + 1][curr.x].visited)
                {
                    sf::Vector2u next(curr.x, curr.y + 1);
                    walls.push_back({curr, next, DOWN, UP});
                    to_add.push_back(next);
                }
                break;
            case LEFT:
                if(curr.x > 0 && !grid[curr.y][curr.x - 1].visited)
                {
                    sf::Vector2u next(curr.x - 1, curr.y);
                    walls.push_back({curr, next, LEFT, RIGHT});
                    to_add.push_back(next);
                }
                break;
            case RIGHT:
                if(curr.x < grid[curr.y].size() - 1 && !grid[curr.y][curr.x + 1].visited)
                {
                    sf::Vector2u next(curr.x + 1, curr.y);
                    walls.push_back({curr, next, RIGHT, LEFT});
                    to_add.push_back(next);
                }
                break;
            }
        }
    }

    // use kruskal's algorithm to join cells into maze
    Disjoint_set<sf::Vector2u> cell_set(cells);

    // shuffle walls
    std::shuffle(walls.begin(), walls.end(), prng);

    std::size_t num_sets = cells.size();
    for(const auto & wall: walls)
    {
        sf::Vector2u set_1 = cell_set.find_rep(wall.cell_1);
        sf::Vector2u set_2 = cell_set.find_rep(wall.cell_2);

        if(set_1 != set_2)
        {
            cell_set.union_reps(set_1, set_2);

            // destroy walls between cells
            grid[wall.cell_1.y][wall.cell_1.x].walls[wall.dir_1] = false;
            grid[wall.cell_2.y][wall.cell_2.x].walls[wall.dir_2] = false;

            if(--num_sets == 1)
                break;
        }
    }
}

