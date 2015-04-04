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

#include <random>

#ifdef __MINGW32__
    #include <ctime>
#endif

#include <gtkmm/application.h>

#include "maze.hpp"
#include "util/logger.hpp"

#ifndef __MINGW32__
    thread_local std::random_device rng;
#endif
thread_local std::mt19937 prng;

int main(int argc, char * argv[])
{
    // TODO: get app name from config
    std::unique_ptr<Tee_log> log(new Tee_log("mazegen_2D.log", std::cerr, Logger::TRACE));
    Logger_locator::init(log.get());

    // random_device curently not working in windows GCC
    #ifdef __MINGW32__
        prng.seed(time(NULL));
    #else
        prng.seed(rng());
    #endif

    // create app and window objects
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.matt.mazegen",
        Gio::APPLICATION_NON_UNIQUE | Gio::APPLICATION_HANDLES_OPEN);

    Logger_locator::get()(Logger::INFO, "Initializing...");
    Maze maze(32, 32);

    Logger_locator::get()(Logger::INFO, "Running...");
    return app->run(maze);
}
