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

#include <string>
#include <stdexcept>

#include <gtkmm/button.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/separator.h>

#include "maze.hpp"

Maze::Maze(const unsigned int width, const unsigned int height):
    _grid_width(Gtk::Adjustment::create(32, 1.0, 100.0, 1.0)),
    _grid_height(Gtk::Adjustment::create(32, 1.0, 100.0, 1.0)),
    _room_attempts(Gtk::Adjustment::create(25, 0.0, 10000.0, 1.0)),
    _wall_rm_attempts(Gtk::Adjustment::create(100, 0.0, 10000.0, 1.0))
{
    set_title("MazeGen 2D"); // TODO: get from config file
    set_default_size(800, 600);

    Gtk::Grid * layout = Gtk::manage(new Gtk::Grid);
    layout->set_border_width(3);
    layout->set_row_spacing(3);
    layout->set_column_spacing(5);
    add(*layout);

    layout->attach(_draw_area, 0, 0, 1, 10);
    _draw_area.set_hexpand();
    _draw_area.set_vexpand();
    _draw_area.signal_draw().connect(sigc::bind(sigc::mem_fun(*this, &Maze::draw), 0, 0));

    layout->attach(*Gtk::manage(new Gtk::Label("Grid width")), 1, 0, 1, 1);
    layout->attach(_grid_width, 2, 0, 1, 1);
    _grid_width.signal_activate().connect(sigc::mem_fun(*this, &Maze::regen));

    layout->attach(*Gtk::manage(new Gtk::Label("Grid height")), 1, 1, 1, 1);
    layout->attach(_grid_height, 2, 1, 1, 1);
    _grid_height.signal_activate().connect(sigc::mem_fun(*this, &Maze::regen));

    layout->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL)), 1, 2, 2, 1);

    layout->attach(*Gtk::manage(new Gtk::Label("Maze algorithm")), 1, 3, 1, 1);
    layout->attach(_mazegen, 2, 3, 1, 1);
    _mazegen.append("Depth First Search");
    _mazegen.append("Prim's Algorithm");
    _mazegen.append("Kruskal's Algorithm");
    _mazegen.set_active_text("Depth First Search");
    _mazegen.signal_changed().connect(sigc::mem_fun(*this, &Maze::regen));

    layout->attach(*Gtk::manage(new Gtk::Label("Room attempts")), 1, 4, 1, 1);
    layout->attach(_room_attempts, 2, 4, 1, 1);
    _room_attempts.signal_activate().connect(sigc::mem_fun(*this, &Maze::regen));

    layout->attach(*Gtk::manage(new Gtk::Label("Wall removal attempts")), 1, 5, 1, 1);
    layout->attach(_wall_rm_attempts, 2, 5, 1, 1);
    _wall_rm_attempts.signal_activate().connect(sigc::mem_fun(*this, &Maze::regen));

    layout->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL)), 1, 6, 2, 1);

    Gtk::Button * regen_butt = Gtk::manage(new Gtk::Button("Regenerate"));
    layout->attach(*regen_butt, 1, 7, 2, 1);
    regen_butt->set_hexpand(false);
    regen_butt->set_halign(Gtk::ALIGN_CENTER);
    regen_butt->signal_clicked().connect(sigc::mem_fun(*this, &Maze::regen));

    Gtk::Label * spacer = Gtk::manage(new Gtk::Label);
    spacer->set_vexpand(true);
    layout->attach(*spacer, 1, 8, 2, 1);

    Gtk::Button * save_butt = Gtk::manage(new Gtk::Button("Save image to disk"));
    layout->attach(*save_butt, 1, 9, 1, 1);
    save_butt->set_hexpand(false);
    save_butt->set_halign(Gtk::ALIGN_END);
    save_butt->signal_clicked().connect(sigc::mem_fun(*this, &Maze::save));

    Gtk::Button * close_butt = Gtk::manage(new Gtk::Button("Close"));
    layout->attach(*close_butt, 2, 9, 1, 1);
    close_butt->set_hexpand(false);
    close_butt->set_halign(Gtk::ALIGN_START);
    close_butt->signal_clicked().connect(sigc::mem_fun(*this, &Maze::hide));

    show_all_children();
    regen();
}

bool Maze::draw(const Cairo::RefPtr<Cairo::Context> & cr, const unsigned int width, const unsigned int height)
{
    double width_d, height_d;

    if(width == 0)
        width_d = (double)_draw_area.get_allocated_width();
    else
        width_d = (double)width;

    if(height == 0)
        height_d = (double)_draw_area.get_allocated_height();
    else
        height_d = (double)height;

    // set background
    cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
    cr->rectangle(0.0, 0.0, width_d, height_d);
    cr->fill_preserve();

    // set line properties
    cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
    cr->set_line_width(2.0);

    cr->stroke();

    double cell_scale_x = width_d / (double)_grid.grid[0].size();
    double cell_scale_y = height_d / (double)_grid.grid.size();

    // draw maze cells
    for(size_t row = 0; row < _grid.grid.size(); ++row)
    {
        for(size_t col = 0; col < _grid.grid[row].size(); ++col)
        {
            sf::Vector2f ul(cell_scale_x * (double)col, cell_scale_y * (double)row);

            if(_grid.grid[row][col].walls[UP])
            {
                cr->move_to(ul.x, ul.y);
                cr->line_to(ul.x + cell_scale_x, ul.y);
            }
            if(_grid.grid[row][col].walls[LEFT])
            {
                cr->move_to(ul.x, ul.y);
                cr->line_to(ul.x, ul.y + cell_scale_y);
            }
        }
    }

    cr->stroke();

    return true;
};

void Maze::regen()
{
    unsigned int grid_width = _grid_width.get_value_as_int();
    unsigned int grid_height = _grid_height.get_value_as_int();
    unsigned int room_attempts = _room_attempts.get_value_as_int();
    unsigned int wall_rm_attempts = _wall_rm_attempts.get_value_as_int();

    Grid::Mazegen_alg mazegen;
    std::string mazegen_txt = _mazegen.get_active_text();
    if(mazegen_txt == "Depth First Search")
        mazegen = Grid::MAZEGEN_DFS;
    else if(mazegen_txt == "Prim's Algorithm")
        mazegen = Grid::MAZEGEN_PRIM;
    else if(mazegen_txt == "Kruskal's Algorithm")
        mazegen = Grid::MAZEGEN_KRUSKAL;
    else
        throw std::invalid_argument(std::string("Unknown maze algorithm: ") + mazegen_txt);

    _grid.init(grid_width, grid_height, mazegen, room_attempts, wall_rm_attempts);

    _draw_area.queue_draw();
}

void Maze::save()
{
    // get image size from user
    Gtk::Dialog size_dialog("Image Size", *this, true);
    size_dialog.add_button("OK", Gtk::RESPONSE_OK);
    size_dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);

    Gtk::Grid size_layout;
    size_layout.set_border_width(3);
    size_layout.set_row_spacing(3);
    size_layout.set_column_spacing(3);
    size_dialog.get_content_area()->add(size_layout);

    size_layout.attach(*Gtk::manage(new Gtk::Label("Width:")), 0, 0, 1, 1);
    Gtk::SpinButton width_spin(Gtk::Adjustment::create(512.0, 1.0, 10000.0));
    size_layout.attach(width_spin, 1, 0, 1, 1);

    size_layout.attach(*Gtk::manage(new Gtk::Label("Height:")), 0, 1, 1, 1);
    Gtk::SpinButton height_spin(Gtk::Adjustment::create(512.0, 1.0, 10000.0));
    size_layout.attach(height_spin, 1, 1, 1, 1);

    size_dialog.show_all_children();

    if(size_dialog.run() != Gtk::RESPONSE_OK)
        return;

    size_dialog.hide();

    Gtk::FileChooserDialog chooser(*this, "Save to", Gtk::FILE_CHOOSER_ACTION_SAVE);
    chooser.set_modal(true);
    chooser.set_current_folder(".");
    chooser.set_select_multiple(false);
    chooser.set_do_overwrite_confirmation(true);
    chooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    chooser.add_button("Save", Gtk::RESPONSE_OK);

    for(const auto & format: Gdk::Pixbuf::get_formats())
    {
        if(!format.is_writable())
            continue;

        Glib::RefPtr<Gtk::FileFilter> image_type = Gtk::FileFilter::create();
        image_type->set_name(format.get_name());

        for(const auto & mimetype: format.get_mime_types())
            image_type->add_mime_type(mimetype);

        chooser.add_filter(image_type);

        if(format.get_name() == "png")
            chooser.set_filter(image_type);
    }

    if(chooser.run() != Gtk::RESPONSE_OK)
        return;

    chooser.hide();

    unsigned int width = width_spin.get_value_as_int();
    unsigned int height = height_spin.get_value_as_int();

    Cairo::RefPtr<Cairo::ImageSurface> render_target = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
    draw(Cairo::Context::create(render_target), width, height);

    try
    {
        Gdk::Pixbuf::create(render_target, 0, 0, width, height)->
            save(chooser.get_filename(), chooser.get_filter()->get_name());
    }
    catch(const Glib::Error & e)
    {
        Gtk::MessageDialog error_box(*this, std::string("Error saving to ") + chooser.get_filename(),
            false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        error_box.set_secondary_text(e.what());
        error_box.run();
    }
}
