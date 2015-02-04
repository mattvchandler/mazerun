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

#include <SFML/System.hpp>

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/spinbutton.h>

#include "maze.hpp"

Maze::Maze(const unsigned int width, const unsigned int height):
    _grid(width, height),
    _mazegen(Grid::MAZEGEN_DFS),
    _room_attempts(25),
    _wall_rm_attempts(100)
{
    set_title("MazeGen 2D"); // TODO: get from config file
    set_default_size(800, 600);

    regen();

    Gtk::Grid * layout = Gtk::manage(new Gtk::Grid);
    add(*layout);

    Gtk::Button * regen_butt = Gtk::manage(new Gtk::Button("Regenerate"));
    layout->attach(*regen_butt, 1, 0, 1, 1);
    regen_butt->signal_clicked().connect(sigc::mem_fun(*this, &Maze::regen));

    _draw_area.set_hexpand();
    _draw_area.set_vexpand();
    layout->attach(_draw_area, 0, 0, 1, 1);
    _draw_area.signal_draw().connect(sigc::mem_fun(*this, &Maze::draw));

    show_all_children();
}

bool Maze::draw(const Cairo::RefPtr<Cairo::Context> & cr)
{
    sf::Vector2u grid_size(_grid.grid[0].size(), _grid.grid.size());
    sf::Vector2f cell_scale((float)_draw_area.get_allocated_width() / (float)grid_size.x,
        (float)_draw_area.get_allocated_height() / (float)grid_size.y);

    // set line properties
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->set_line_width(2.0f);

    // draw border
    cr->move_to(0.0f, 0.0f);
    cr->line_to((float)_draw_area.get_allocated_width(), 0.0f);

    cr->move_to((float)_draw_area.get_allocated_width(), 0.0f);
    cr->line_to((float)_draw_area.get_allocated_width(), (float)_draw_area.get_allocated_height());

    cr->move_to((float)_draw_area.get_allocated_width(), (float)_draw_area.get_allocated_height());
    cr->line_to(0.0f, (float)_draw_area.get_allocated_height());

    cr->move_to(0.0f, (float)_draw_area.get_allocated_height());
    cr->line_to(0.0f, 0.0f);

    // draw maze cells
    for(size_t row = 0; row < grid_size.y; ++row)
    {
        for(size_t col = 0; col < grid_size.x; ++col)
        {
            sf::Vector2f ul(cell_scale.x * (float)col, cell_scale.y * (float)row);

            if(_grid.grid[row][col].walls[UP])
            {
                cr->move_to(ul.x, ul.y);
                cr->line_to(ul.x + cell_scale.x, ul.y);
            }
            if(_grid.grid[row][col].walls[LEFT])
            {
                cr->move_to(ul.x, ul.y);
                cr->line_to(ul.x, ul.y + cell_scale.y);
            }
        }
    }

    cr->stroke();

    return true;
};

void Maze::regen()
{
    _grid.init(_mazegen, _room_attempts, _wall_rm_attempts);

    auto win = _draw_area.get_window();
    if(win)
        win->invalidate(true);
}
