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

#include <atomic>
#include <csignal>

#include "world/world.hpp" // includes SFML, must be included before Xlib

#ifdef __linux
#include <X11/Xlib.h>
#endif

#include "components/audio.hpp"
#include "components/model.hpp"
#include "opengl/texture.hpp"
#include "util/logger.hpp"
#include "util/message.hpp"

std::atomic_bool interrupted(false);

void sigint_handler(int)
{
    interrupted = true;
}

int main(int argc, char * argv[])
{
    #ifdef __linux
    XInitThreads(); // needed for multithreaded window access on Linux
    #endif

    // set up sigint (^C) handler
    std::signal(SIGINT, &sigint_handler);

    // set up logging service
    // TODO: get app name  & log level from config
    std::unique_ptr<Tee_log> log(new Tee_log("mazerun.log", std::cerr, Logger::TRACE));
    Logger_locator::init(log.get());

    Logger_locator::get()(Logger::INFO, "Initializing...");

    // create other services
    std::unique_ptr<Message> message(new Message);
    std::unique_ptr<Model_cache> model_cache(new Model_cache);
    std::unique_ptr<Texture_cache> texture_cache(new Texture_cache);
    std::unique_ptr<Jukebox> jukebox(new Jukebox);

    Message_locator::init(message.get());
    Model_cache_locator::init(model_cache.get());
    Texture_cache_locator::init(texture_cache.get());
    Jukebox_locator::init(jukebox.get());

    // initialize world - using a pointer so we can destroy it manually
    std::unique_ptr<World> world(new World);

    Logger_locator::get()(Logger::INFO, "Running...");
    world->game_loop();

    Logger_locator::get()(Logger::INFO, "Deinitializing...");
    world.reset();

    // destroy services
    Jukebox_locator::init(nullptr);
    Texture_cache_locator::init(nullptr);
    Model_cache_locator::init(nullptr);
    Message_locator::init(nullptr);

    message.reset();
    model_cache.reset();
    texture_cache.reset();
    jukebox.reset();

    // destroy log service
    Logger_locator::get()(Logger::INFO, "Shutdown...");
    Logger_locator::init(nullptr);
    log.reset();

    return EXIT_SUCCESS;
}
