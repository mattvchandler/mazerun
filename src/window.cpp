// window.cpp
// main window code

// Copyright 2015 Matthew Chandler

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>

#include <GL/glew.h>

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>

int main(int argc, char * argv[])
{
    sf::Window win(sf::VideoMode(800, 600), "mazerun", sf::Style::Default, sf::ContextSettings(24, 8, 8, 3, 0));

    std::cout<<"V: "<<win.getSettings().majorVersion<<"."<<win.getSettings().minorVersion<<std::endl;
    std::cout<<"D: "<<win.getSettings().depthBits<<std::endl;
    std::cout<<"S: "<<win.getSettings().stencilBits<<std::endl;
    std::cout<<"A: "<<win.getSettings().antialiasingLevel<<std::endl;

    // initialize glew
    if(glewInit() != GLEW_OK)
    {
        std::cerr<<"Error loading glew"<<std::endl;
        return EXIT_FAILURE;
    }

    // set clear color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    bool running = true;
    while(running)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        win.display();

        sf::Event ev;
        while(win.pollEvent(ev))
        {
            switch(ev.type)
            {
            case sf::Event::Closed:
                win.close();
                running = false;
                break;
            default:
                break;
            }
        }
    }

    return 0;
}
