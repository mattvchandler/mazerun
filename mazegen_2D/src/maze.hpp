// maze.hpp
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

#ifndef MAZE_HPP
#define MAZE_HPP

#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/window.h>

#include "grid.hpp"

class Maze final: public Gtk::Window
{
public:
    Maze(const unsigned int width, const unsigned int height);
private:
    bool draw(const Cairo::RefPtr<Cairo::Context> & cr, const unsigned int width, const unsigned int height);
    void regen();

    Grid _grid;

    Gtk::DrawingArea _draw_area;

    Gtk::SpinButton _grid_width;
    Gtk::SpinButton _grid_height;
    Gtk::ComboBoxText _mazegen;
    Gtk::SpinButton _room_attempts;
    Gtk::SpinButton _wall_rm_attempts;
};

#endif // MAZE_HPP
