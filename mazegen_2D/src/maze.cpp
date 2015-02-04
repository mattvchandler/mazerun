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
#include <gtkmm/label.h>
#include <gtkmm/separator.h>

#include "maze.hpp"

Maze::Maze(const unsigned int width, const unsigned int height):
    _grid_width(32),
    _grid_height(32),
    _mazegen(Grid::MAZEGEN_DFS),
    _room_attempts(25),
    _wall_rm_attempts(100),
    _grid_width_spin(Gtk::Adjustment::create(_grid_width, 0.0, 100.0, 1.0), 1.0, 0),
    _grid_height_spin(Gtk::Adjustment::create(_grid_height, 0.0, 100.0, 1.0), 1.0, 0),
    _room_attempts_spin(Gtk::Adjustment::create(_room_attempts, 0.0, 10000.0, 1.0), 1.0, 0),
    _wall_rm_attempts_spin(Gtk::Adjustment::create(_wall_rm_attempts, 0.0, 10000.0, 1.0), 1.0, 0)
{
    set_title("MazeGen 2D"); // TODO: get from config file
    set_default_size(800, 600);

    regen();

    Gtk::Grid * layout = Gtk::manage(new Gtk::Grid);
    layout->set_border_width(3);
    layout->set_row_spacing(3);
    layout->set_column_spacing(5);
    add(*layout);

    layout->attach(_draw_area, 0, 0, 1, 8);
    _draw_area.set_hexpand();
    _draw_area.set_vexpand();
    _draw_area.signal_draw().connect(sigc::mem_fun(*this, &Maze::draw));

    layout->attach(*Gtk::manage(new Gtk::Label("Grid width")), 1, 0, 1, 1);
    layout->attach(_grid_width_spin, 2, 0, 1, 1);

    layout->attach(*Gtk::manage(new Gtk::Label("Grid height")), 1, 1, 1, 1);
    layout->attach(_grid_height_spin, 2, 1, 1, 1);

    layout->attach(*Gtk::manage(new Gtk::Label("Room attempts")), 1, 2, 1, 1);
    layout->attach(_room_attempts_spin, 2, 2, 1, 1);

    layout->attach(*Gtk::manage(new Gtk::Label("Wall removal attempts")), 1, 3, 1, 1);
    layout->attach(_wall_rm_attempts_spin, 2, 3, 1, 1);

    layout->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL)), 1, 4, 2, 1);

    Gtk::Button * regen_butt = Gtk::manage(new Gtk::Button("Regenerate"));
    layout->attach(*regen_butt, 1, 5, 2, 1);
    regen_butt->set_hexpand(false);
    regen_butt->set_halign(Gtk::ALIGN_CENTER);
    regen_butt->signal_clicked().connect(sigc::mem_fun(*this, &Maze::regen));

    Gtk::Label * spacer = Gtk::manage(new Gtk::Label);
    spacer->set_vexpand(true);
    layout->attach(*spacer, 1, 6, 2, 1);

    Gtk::Button * save_butt = Gtk::manage(new Gtk::Button("Save to disk"));
    layout->attach(*save_butt, 1, 7, 1, 1);
    save_butt->set_hexpand(false);
    save_butt->set_halign(Gtk::ALIGN_END);
    save_butt->signal_clicked().connect(sigc::mem_fun(*this, &Maze::hide));

    Gtk::Button * close_butt = Gtk::manage(new Gtk::Button("Close"));
    layout->attach(*close_butt, 2, 7, 1, 1);
    close_butt->set_hexpand(false);
    close_butt->set_halign(Gtk::ALIGN_START);
    close_butt->signal_clicked().connect(sigc::mem_fun(*this, &Maze::hide));

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
    _grid.init(_grid_width, _grid_height, _mazegen, _room_attempts, _wall_rm_attempts);

    auto win = _draw_area.get_window();
    if(win)
        win->invalidate(true);
}
