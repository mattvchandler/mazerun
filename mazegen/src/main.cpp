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
#include <vector>

#include <cstdlib>
#include <ctime>

#include <SFML/Graphics.hpp>

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
    for(size_t i = 0; i < 4; ++i)
        walls[i] = true;
}

class Maze_grid
{
public:
    Maze_grid(const sf::Vector2u & grid_size);
    void init();
    sf::Vector2u size() const;
    Grid_cell & operator[](const sf::Vector2u & cell);
    Grid_cell operator[](const sf::Vector2u & cell) const;
private:
    void mazegen_dfs(const sf::Vector2u & start);
    void mazegen_prim(const sf::Vector2u & start);
    std::vector<std::vector<Grid_cell>> _grid;
};

Maze_grid::Maze_grid(const sf::Vector2u & grid_size)
    :_grid(grid_size.y, std::vector<Grid_cell>(grid_size.x))
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
    mazegen_prim(sf::Vector2u(0, 0));
}

sf::Vector2u Maze_grid::size() const
{
    return sf::Vector2u(_grid.size(), _grid[0].size());
}

Grid_cell & Maze_grid::operator[](const sf::Vector2u & cell)
{
    return _grid[cell.y][cell.x];
}
Grid_cell Maze_grid::operator[](const sf::Vector2u & cell) const
{
    return _grid[cell.y][cell.x];
}

void Maze_grid::mazegen_dfs(const sf::Vector2u & start)
{
    _grid[start.y][start.x].visited = true;

    // shuffle directions list
    Direction dirs[4] = {UP, DOWN, LEFT, RIGHT};
    for(size_t i = 0; i < 4; ++i)
    {
        std::swap(dirs[i], dirs[rand() % 4]);
    }

    for(size_t i = 0; i < 4; ++i)
    {
        switch(dirs[i])
        {
        case UP:
            if(start.y > 0 && !_grid[start.y - 1][start.x].visited)
            {
                _grid[start.y][start.x].walls[UP] = false;
                _grid[start.y - 1][start.x].walls[DOWN] = false;
                mazegen_dfs(sf::Vector2u(start.x, start.y - 1));
            }
            break;
        case DOWN:
            if(start.y < _grid.size() - 1 && !_grid[start.y + 1][start.x].visited)
            {
                _grid[start.y][start.x].walls[DOWN] = false;
                _grid[start.y + 1][start.x].walls[UP] = false;
                mazegen_dfs(sf::Vector2u(start.x, start.y + 1));
            }
            break;
        case LEFT:
            if(start.x > 0 && !_grid[start.y][start.x - 1].visited)
            {
                _grid[start.y][start.x].walls[LEFT] = false;
                _grid[start.y][start.x - 1].walls[RIGHT] = false;
                mazegen_dfs(sf::Vector2u(start.x - 1, start.y));
            }
            break;
        case RIGHT:
            if(start.x < _grid[start.y].size() - 1 && !_grid[start.y][start.x + 1].visited)
            {
                _grid[start.y][start.x].walls[RIGHT] = false;
                _grid[start.y][start.x + 1].walls[LEFT] = false;
                mazegen_dfs(sf::Vector2u(start.x + 1, start.y));
            }
            break;
        }
    }
}

void Maze_grid::mazegen_prim(const sf::Vector2u & start)
{
    std::vector<std::pair<sf::Vector2u, Direction>> walls;

    auto add_cell = [& walls, this](const sf::Vector2u & cell)
    {
        for(size_t i = 0; i < 4; ++i)
        {
            switch(i)
            {
            case UP:
                if(cell.y > 0 && !_grid[cell.y - 1][cell.x].visited)
                {
                    walls.push_back(std::make_pair(cell, UP));
                }
                break;
            case DOWN:
                if(cell.y < _grid.size() - 1 && !_grid[cell.y + 1][cell.x].visited)
                {
                    walls.push_back(std::make_pair(cell, DOWN));
                }
                break;
            case LEFT:
                if(cell.x > 0 && !_grid[cell.y][cell.x - 1].visited)
                {
                    walls.push_back(std::make_pair(cell, LEFT));
                }
                break;
            case RIGHT:
                if(cell.x < _grid[cell.y].size() - 1 && !_grid[cell.y][cell.x + 1].visited)
                {
                    walls.push_back(std::make_pair(cell, RIGHT));
                }
                break;
            }
        }
    };

    add_cell(start);

    _grid[start.y][start.x].visited = true;

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
            if(curr.y > 0 && !_grid[curr.y - 1][curr.x].visited)
            {
                _grid[curr.y][curr.x].walls[UP] = false;
                _grid[curr.y - 1][curr.x].walls[DOWN] = false;
                _grid[curr.y - 1][curr.x].visited = true;
                next = sf::Vector2u(curr.x, curr.y - 1);
                next_found = true;
            }
            break;
        case DOWN:
            if(curr.y < _grid.size() - 1 && !_grid[curr.y + 1][curr.x].visited)
            {
                _grid[curr.y][curr.x].walls[DOWN] = false;
                _grid[curr.y + 1][curr.x].walls[UP] = false;
                _grid[curr.y + 1][curr.x].visited = true;
                next = sf::Vector2u(curr.x, curr.y + 1);
                next_found = true;
            }
            break;
        case LEFT:
            if(curr.x > 0 && !_grid[curr.y][curr.x - 1].visited)
            {
                _grid[curr.y][curr.x].walls[LEFT] = false;
                _grid[curr.y][curr.x - 1].walls[RIGHT] = false;
                _grid[curr.y][curr.x - 1].visited = true;
                next = sf::Vector2u(curr.x - 1, curr.y);
                next_found = true;
            }
            break;
        case RIGHT:
            if(curr.x < _grid[curr.y].size() - 1 && !_grid[curr.y][curr.x + 1].visited)
            {
                _grid[curr.y][curr.x].walls[RIGHT] = false;
                _grid[curr.y][curr.x + 1].walls[LEFT] = false;
                _grid[curr.y][curr.x + 1].visited = true;
                next = sf::Vector2u(curr.x + 1, curr.y);
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

    sf::Vector2u _grid_size = _grid.size();
    sf::Vector2f cell_scale((float)_win.getSize().x / (float)_grid_size.x, (float)_win.getSize().y / (float)_grid_size.y);

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
    for(size_t row = 0; row < _grid_size.y; ++row)
    {
        for(size_t col = 0; col < _grid_size.x; ++col)
        {
            sf::Vector2f ul(cell_scale.x * (float)col, cell_scale.y * (float)row);

            if(_grid[sf::Vector2u(col, row)].walls[UP])
            {
                _lines.append(sf::Vertex(ul, sf::Color::Black));
                _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y), sf::Color::Black));
            }
            // if(_grid[sf::Vector2u(col, row)].walls[DOWN])
            // {
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x, ul.y + cell_scale.y), sf::Color::Black));
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y + cell_scale.y), sf::Color::Black));
            // }
            if(_grid[sf::Vector2u(col, row)].walls[LEFT])
            {
                _lines.append(sf::Vertex(ul, sf::Color::Black));
                _lines.append(sf::Vertex(sf::Vector2f(ul.x, ul.y + cell_scale.y), sf::Color::Black));
            }
            // if(_grid[sf::Vector2u(col, row)].walls[RIGHT])
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
