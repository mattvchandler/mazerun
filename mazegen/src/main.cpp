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

#include <SFML/Graphics.hpp>

class Maze
{
public:
    Maze(sf::RenderWindow & win, const sf::Vector2u & grid_size);
    void init();
    void draw();
    void set_grid_size(const sf::Vector2u & grid_size);
    sf::Vector2u get_grid_size() const;
private:
    sf::RenderWindow & _win;
    sf::VertexArray _lines;

    sf::Vector2u _grid_size;
};

Maze::Maze(sf::RenderWindow & win, const sf::Vector2u & grid_size):
    _win(win),
    _lines(sf::Lines),
    _grid_size(grid_size)
{
}

void Maze::init()
{
}

void Maze::draw()
{
    _win.draw(_lines);
}

int main(int argc, char * argv[])
{
    sf::RenderWindow win(sf::VideoMode(800, 600), "mazegen", sf::Style::Default);

    Maze maze(win, sf::Vector2u(32, 32));

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
