// maze.cpp
// 2D maze data & routines

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

#include "maze.hpp"

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
            if(_grid.grid[row][col].walls[LEFT])
            {
                _lines.append(sf::Vertex(ul, sf::Color::Black));
                _lines.append(sf::Vertex(sf::Vector2f(ul.x, ul.y + cell_scale.y), sf::Color::Black));
            }
        }
    }
}

void Maze::draw()
{
    _win.draw(_lines);
}

