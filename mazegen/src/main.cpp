// main.cpp
// main entry point

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

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <vector>

#include <cstdlib>
#include <ctime>

#include <SFML/Graphics.hpp>

// TODO: replace rand with C++ random lib, cuse stl shuffle instead of writing my own

enum Direction {UP = 0, DOWN, LEFT, RIGHT};

class Grid_cell
{
public:
    Grid_cell();
    bool walls[4];
    bool visited;
};

Grid_cell::Grid_cell(): visited(false)
{
    for(int i = 0; i < 4; ++i)
        walls[i] = true;
}

class Maze_grid
{
public:
    Maze_grid(const sf::Vector2u & grid_size);
    void init();

    std::vector<std::vector<Grid_cell>> grid;
private:
    void mazegen_dfs(const sf::Vector2u & start);
    void mazegen_prim(const sf::Vector2u & start);
    void mazegen_kruskal(const sf::Vector2u & start);
};

Maze_grid::Maze_grid(const sf::Vector2u & grid_size)
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

void Maze_grid::init()
{
    mazegen_dfs(sf::Vector2u(0, 0));
}

void Maze_grid::mazegen_dfs(const sf::Vector2u & start)
{
    if(grid[start.y][start.x].visited)
        return;

    grid[start.y][start.x].visited = true;

    // shuffle directions list
    Direction dirs[4] = {UP, DOWN, LEFT, RIGHT};
    for(int i = 0; i < 4; ++i) // TODO: stl shuffle?
    {
        std::swap(dirs[i], dirs[rand() % 4]);
    }

    for(int i = 0; i < 4; ++i)
    {
        switch(dirs[i])
        {
        case UP:
            if(start.y > 0 && !grid[start.y - 1][start.x].visited)
            {
                grid[start.y][start.x].walls[UP] = false;
                grid[start.y - 1][start.x].walls[DOWN] = false;
                mazegen_dfs(sf::Vector2u(start.x, start.y - 1));
            }
            break;
        case DOWN:
            if(start.y < grid.size() - 1 && !grid[start.y + 1][start.x].visited)
            {
                grid[start.y][start.x].walls[DOWN] = false;
                grid[start.y + 1][start.x].walls[UP] = false;
                mazegen_dfs(sf::Vector2u(start.x, start.y + 1));
            }
            break;
        case LEFT:
            if(start.x > 0 && !grid[start.y][start.x - 1].visited)
            {
                grid[start.y][start.x].walls[LEFT] = false;
                grid[start.y][start.x - 1].walls[RIGHT] = false;
                mazegen_dfs(sf::Vector2u(start.x - 1, start.y));
            }
            break;
        case RIGHT:
            if(start.x < grid[start.y].size() - 1 && !grid[start.y][start.x + 1].visited)
            {
                grid[start.y][start.x].walls[RIGHT] = false;
                grid[start.y][start.x + 1].walls[LEFT] = false;
                mazegen_dfs(sf::Vector2u(start.x + 1, start.y));
            }
            break;
        }
    }
}

void Maze_grid::mazegen_prim(const sf::Vector2u & start)
{
    if(grid[start.y][start.x].visited)
        return;

    std::vector<std::pair<sf::Vector2u, Direction>> walls;

    auto add_cell = [& walls, this](const sf::Vector2u & cell)
    {
        for(int i = 0; i < 4; ++i)
        {
            switch(i)
            {
            case UP:
                if(cell.y > 0 && !grid[cell.y - 1][cell.x].visited)
                {
                    walls.push_back(std::make_pair(cell, UP));
                }
                break;
            case DOWN:
                if(cell.y < grid.size() - 1 && !grid[cell.y + 1][cell.x].visited)
                {
                    walls.push_back(std::make_pair(cell, DOWN));
                }
                break;
            case LEFT:
                if(cell.x > 0 && !grid[cell.y][cell.x - 1].visited)
                {
                    walls.push_back(std::make_pair(cell, LEFT));
                }
                break;
            case RIGHT:
                if(cell.x < grid[cell.y].size() - 1 && !grid[cell.y][cell.x + 1].visited)
                {
                    walls.push_back(std::make_pair(cell, RIGHT));
                }
                break;
            }
        }
    };

    add_cell(start);

    grid[start.y][start.x].visited = true;

    while(walls.size() > 0)
    {
        size_t curr_ind = rand() % walls.size();
        auto curr_it = walls.begin() + curr_ind;

        sf::Vector2u curr = curr_it->first, next;
        Direction dir = curr_it->second;
        bool next_found = false;

        switch(dir)
        {
        case UP:
            if(curr.y > 0 && !grid[curr.y - 1][curr.x].visited)
            {
                next = sf::Vector2u(curr.x, curr.y - 1);
                grid[curr.y][curr.x].walls[UP] = false;
                grid[next.y][next.x].walls[DOWN] = false;
                grid[next.y][next.x].visited = true;
                next_found = true;
            }
            break;
        case DOWN:
            if(curr.y < grid.size() - 1 && !grid[curr.y + 1][curr.x].visited)
            {
                next = sf::Vector2u(curr.x, curr.y + 1);
                grid[curr.y][curr.x].walls[DOWN] = false;
                grid[next.y][next.x].walls[UP] = false;
                grid[next.y][next.x].visited = true;
                next_found = true;
            }
            break;
        case LEFT:
            if(curr.x > 0 && !grid[curr.y][curr.x - 1].visited)
            {
                next = sf::Vector2u(curr.x - 1, curr.y);
                grid[curr.y][curr.x].walls[LEFT] = false;
                grid[next.y][next.x].walls[RIGHT] = false;
                grid[next.y][next.x].visited = true;
                next_found = true;
            }
            break;
        case RIGHT:
            if(curr.x < grid[curr.y].size() - 1 && !grid[curr.y][curr.x + 1].visited)
            {
                next = sf::Vector2u(curr.x + 1, curr.y);
                grid[curr.y][curr.x].walls[RIGHT] = false;
                grid[next.y][next.x].walls[LEFT] = false;
                grid[next.y][next.x].visited = true;
                next_found = true;
            }
            break;
        }

        walls.erase(curr_it);

        if(next_found)
        {
            add_cell(next);
        }
    }
}

template <typename T>
class Disjoint_set
{
public:
    Disjoint_set(const std::vector<T> & items);
    const T & find_rep(const T & a) const;
    void union_reps(const T & a, const T & b);
private:
    std::unordered_map<T, std::pair<T, unsigned int>> _set;
};

template <typename T>
Disjoint_set<T>::Disjoint_set(const std::vector<T> & items)
{
    for(const auto & i: items)
    {
        _set[i] = std::make_pair(i, 0);
    }
}

template <typename T>
const T & Disjoint_set<T>::find_rep(const T & a) const
{
    const T * x = &_set.at(a).first;

    if(a != *x)
        return find_rep(*x);
    else
        return a;
}

template <typename T>
void Disjoint_set<T>::union_reps(const T & a, const T & b)
{
    T a_root = find_rep(a);
    T b_root = find_rep(b);

    if(a_root == b_root)
        return;

    // compare ranks
    if(_set[a_root].second < _set[b_root].second)
        _set[a_root].first = b_root;
    else if(_set[a_root].second > _set[b_root].second)
        _set[b_root].first = a_root;
    else // equal ranks
    {
        _set[b_root].first = a_root;
        ++_set[a_root].second;
    }
}

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

void Maze_grid::mazegen_kruskal(const sf::Vector2u & start)
{
    if(grid[start.y][start.x].visited)
        return;

    // floodfill to get all cells connected, add walls to list, cells to disjoint set, mark cell visited
    std::vector<std::tuple<sf::Vector2u, sf::Vector2u, Direction, Direction>> walls;
    std::vector<sf::Vector2u> cells;
    std::vector<sf::Vector2u> to_add = {start};

    while(to_add.size() > 0)
    {
        sf::Vector2u curr = to_add.back();
        to_add.pop_back();

        if(grid[curr.y][curr.x].visited)
            continue;

        grid[curr.y][curr.x].visited = true;
        cells.push_back(curr);

        for(int i = 0; i < 4; ++i)
        {
            switch(i)
            {
            case UP:
                if(curr.y > 0 && !grid[curr.y - 1][curr.x].visited)
                {
                    sf::Vector2u next(curr.x, curr.y - 1);
                    walls.push_back(std::make_tuple(curr, next, UP, DOWN));
                    to_add.push_back(next);
                }
                break;
            case DOWN:
                if(curr.y < grid.size() - 1 && !grid[curr.y + 1][curr.x].visited)
                {
                    sf::Vector2u next(curr.x, curr.y + 1);
                    walls.push_back(std::make_tuple(curr, next, DOWN, UP));
                    to_add.push_back(next);
                }
                break;
            case LEFT:
                if(curr.x > 0 && !grid[curr.y][curr.x - 1].visited)
                {
                    sf::Vector2u next(curr.x - 1, curr.y);
                    walls.push_back(std::make_tuple(curr, next, LEFT, RIGHT));
                    to_add.push_back(next);
                }
                break;
            case RIGHT:
                if(curr.x < grid[curr.y].size() - 1 && !grid[curr.y][curr.x + 1].visited)
                {
                    sf::Vector2u next(curr.x + 1, curr.y);
                    walls.push_back(std::make_tuple(curr, next, RIGHT, LEFT));
                    to_add.push_back(next);
                }
                break;
            }
        }
    }

    Disjoint_set<sf::Vector2u> cell_set(cells);

    // shuffle walls
    for(size_t i = 0; i < walls.size(); ++i)
    {
        std::swap(walls[i], walls[rand() % walls.size()]);
    }

    // for wall in walls, and while # sets > 1
    size_t num_sets = cells.size();
    for(const auto & wall: walls)
    {
        sf::Vector2u a, b;
        Direction a_dir, b_dir;
        std::tie(a, b, a_dir, b_dir) = wall;

        sf::Vector2u set_a = cell_set.find_rep(a);
        sf::Vector2u set_b = cell_set.find_rep(b);

        if(set_a != set_b)
        {
            cell_set.union_reps(set_a, set_b);
            grid[a.y][a.x].walls[a_dir] = false;
            grid[b.y][b.x].walls[b_dir] = false;
            if(--num_sets == 1)
                break;
        }
    }
}

class Maze
{
public:
    Maze(sf::RenderWindow & win, const sf::Vector2u & grid_size);
    void init();
    void draw();
    void set_grid_size(const sf::Vector2u & grid_size);
    sf::Vector2u get_grid_size() const;
private:
    Maze_grid _grid;
    sf::RenderWindow & _win;
    sf::VertexArray _lines;
};

Maze::Maze(sf::RenderWindow & win, const sf::Vector2u & grid_size):
    _grid(grid_size),
    _win(win),
    _lines(sf::Lines)
{
}

void Maze::init()
{
    _grid.init();

    sf::Vector2u grid_size(_grid.grid[0].size(), _grid.grid.size());
    sf::Vector2f cell_scale((float)_win.getSize().x / (float)grid_size.x, (float)_win.getSize().y / (float)grid_size.y);

    // draw border
    _lines.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color::Black));
    _lines.append(sf::Vertex(sf::Vector2f((float)_win.getSize().x, 0.0f), sf::Color::Black));

    _lines.append(sf::Vertex(sf::Vector2f((float)_win.getSize().x, 0.0f), sf::Color::Black));
    _lines.append(sf::Vertex(sf::Vector2f((float)_win.getSize().x, (float)_win.getSize().y), sf::Color::Black));

    _lines.append(sf::Vertex(sf::Vector2f((float)_win.getSize().x, (float)_win.getSize().y), sf::Color::Black));
    _lines.append(sf::Vertex(sf::Vector2f(0.0f, (float)_win.getSize().y), sf::Color::Black));

    _lines.append(sf::Vertex(sf::Vector2f(0.0f, (float)_win.getSize().y), sf::Color::Black));
    _lines.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color::Black));

    // draw maze cells
    for(size_t row = 0; row < grid_size.y; ++row)
    {
        for(size_t col = 0; col < grid_size.x; ++col)
        {
            sf::Vector2f ul(cell_scale.x * (float)col, cell_scale.y * (float)row);

            if(_grid.grid[row][col].walls[UP])
            {
                _lines.append(sf::Vertex(ul, sf::Color::Black));
                _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y), sf::Color::Black));
            }
            // if(_grid.grid[row][col].walls[DOWN])
            // {
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x, ul.y + cell_scale.y), sf::Color::Black));
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y + cell_scale.y), sf::Color::Black));
            // }
            if(_grid.grid[row][col].walls[LEFT])
            {
                _lines.append(sf::Vertex(ul, sf::Color::Black));
                _lines.append(sf::Vertex(sf::Vector2f(ul.x, ul.y + cell_scale.y), sf::Color::Black));
            }
            // if(_grid.grid[row][col].walls[RIGHT])
            // {
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y), sf::Color::Black));
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y + cell_scale.y), sf::Color::Black));
            // }
        }
    }
}

void Maze::draw()
{
    _win.draw(_lines);
}

int main(int argc, char * argv[])
{
    srand(time(NULL));
    sf::RenderWindow win(sf::VideoMode(800, 600), "mazegen", sf::Style::Default);

    Maze maze(win, sf::Vector2u(32, 32));
    maze.init();

    while(win.isOpen())
    {
        win.clear(sf::Color(255, 255, 255, 255));
        maze.draw();
        win.display();
        sf::Event ev;

        if(win.waitEvent(ev))
        {
            switch(ev.type)
            {
                case sf::Event::Closed:
                    win.close();
                    break;
                case sf::Event::Resized:
                    // resize();
                    break;
                default:
                    break;
            }
        }
    }

    return EXIT_SUCCESS;
}
