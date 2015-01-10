// window.cpp
// main window code

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

#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#ifdef __linux
#include <X11/Xlib.h>
#endif

// TODO: split this file (many times)

// random floating point number
float randf()
{
    return (float)(rand()) / RAND_MAX;
}

// check for OpenGL error and print message
void check_error(const char * at)
{
    GLenum e = glGetError();
    if(e == GL_NO_ERROR)
        return;
    std::cerr<<"OpenGL Error at "<<at<<": "<<gluErrorString(e)<<std::endl;
}

class Entity
{
public:
    Entity();

    // set / reset pos / orientation
    // default args put at origin looking down Y axis with Z axis up
    void set(const glm::vec3 & pos = glm::vec3(0.0f),
        const glm::vec3 & forward = glm::vec3(0.0f, 1.0f, 0.0f),
        const glm::vec3 & up = glm::vec3(0.0f, 0.0f, 1.0f));

    // move the position acoording to a vector
    void translate(const glm::vec3 & translation);

    // rotate around an axis
    // angles in radians
    void rotate(const float angle, const glm::vec3 & axis);

    glm::vec3 pos() const;
    glm::vec3 forward() const;
    glm::vec3 up() const;
    glm::vec3 right() const;
protected:
    glm::vec3 _pos;
    glm::vec3 _forward; // TODO: store as matrix and calc vectors as needed?
    glm::vec3 _up;
};


glm::vec3 Entity::pos() const
{
    return _pos;
}

glm::vec3 Entity::forward() const
{
    return _forward;
}

glm::vec3 Entity::up() const
{
    return _up;
}

glm::vec3 Entity::right() const
{
    return glm::cross(_forward, _up);
}

Entity::Entity()
{
    set();
}

// set / reset pos / orientation
void Entity::set(const glm::vec3 & pos, const glm::vec3 & forward,
    const glm::vec3 & up)
{
    _pos = pos;
    _forward = glm::normalize(forward);
    _up = glm::normalize(up);
}

// move the position acoording to a vector
void Entity::translate(const glm::vec3 & translation)
{
    _pos += translation;
}

// rotate around an axis
// angles in radians
void Entity::rotate(const float angle, const glm::vec3 & axis)
{
    // using Rodrigues' rotation formula
    // http://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
    _forward = glm::normalize(_forward * cosf(angle) + glm::cross(axis, _forward) * sinf(angle)
        + axis * glm::dot(axis, _forward) * (1.0f - cosf(angle)));
    _up = glm::normalize(_up * cosf(angle) + glm::cross(axis, _up) * sinf(angle)
        + axis * glm::dot(axis, _up) * (1.0f - cosf(angle)));
}

class Camera: public Entity // TODO: is this class needed?
{
public:
    glm::mat4 view_mat() const;
};

glm::mat4 Camera::view_mat() const
{
    return glm::lookAt(_pos, _pos + _forward, _up);
}

class Player: public Camera
{
public:
    void handle_input(const sf::Window & win, const float dt);
};

void Player::handle_input(const sf::Window & win, const float dt)
{
    static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;
    static sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(win);

    float mov_scale = 0.1f;
    sf::Vector2i new_mouse_pos = sf::Mouse::getPosition(win);
    // TODO: invalidate signal somehow

    // move forward
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        translate(mov_scale * forward() * dt);
    }

    // move backwards
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        translate(-mov_scale * forward() * dt);
    }

    // move left
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        translate(-mov_scale * right() * dt);
    }

    // move right
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        translate(mov_scale * right() * dt);
    }

    // TODO: replace with jump?
    // move up
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
    {
        translate(mov_scale * glm::vec3(0.0f, 0.0f, 1.0f) * dt);
    }

    // move down
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
    {
        translate(-mov_scale * glm::vec3(0.0f, 0.0f, 1.0f) * dt);
    }

    // rotate view with mouse click & drag
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        int d_x = new_mouse_pos.x - old_mouse_pos.x;
        int d_y = new_mouse_pos.y - old_mouse_pos.y;

        rotate(0.001f * d_y, right());
        rotate(0.001f * d_x, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    old_mouse_pos = new_mouse_pos;
}

class Texture
{
public:
    Texture();
    ~Texture();
    virtual void bind() = 0;
protected:
    GLuint _texid;
};

Texture::Texture(): _texid(0)
{
}

Texture::~Texture()
{
    if(_texid)
    {
        glDeleteTextures(1, &_texid);
    }
}

class Texture_cubemap: public Texture
{
public:
    void init(const std::string & left_fname, const std::string & right_fname,
    const std::string & back_fname, const std::string & front_fname,
    const std::string & down_fname, const std::string & up_fname);
    void bind();
};

// TODO should all inits should return bool OR throw (pick one)
// create a cubemap texture from 6 filenames
void Texture_cubemap::init(const std::string & left_fname, const std::string & right_fname,
    const std::string & back_fname, const std::string & front_fname,
    const std::string & down_fname, const std::string & up_fname)
{
    // create array of pairs: filename with type enum
    std::vector<std::pair<std::string, GLenum>> filenames =
    {
        std::make_pair(left_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_X),
        std::make_pair(right_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_X),
        std::make_pair(back_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z),
        std::make_pair(front_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_Z),
        std::make_pair(down_fname, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y),
        std::make_pair(up_fname, GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
    };

    glGenTextures(1, &_texid);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texid);

    for(const auto & filename: filenames)
    {
        // load each file
        sf::Image img;
        if(!img.loadFromFile(filename.first))
        {
            throw std::ios_base::failure(std::string("Error reading image file: ") + filename.first);
        }

        // send data to OpenGL
        glTexImage2D(filename.second, 0, GL_RGBA, img.getSize().x, img.getSize().y,
            0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
    }

    // set params
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Texture_cubemap::bind()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texid);
}

class Shader_prog
{
public:
    Shader_prog();
    ~Shader_prog();
    void init(const std::vector<std::pair<std::string, GLenum>> & sources,
        const std::vector<std::pair<std::string, GLuint>> & attribs);
    bool add_uniform(const std::string & uniform);
    GLuint operator()() const;
    // TODO: probably my own exceptions, rather than use system exceptions

    std::unordered_map<std::string, GLuint> uniforms; // convenience storage for uniform values // TODO: maybe make private and add get method?
protected:
    GLuint _prog;
};

Shader_prog::Shader_prog(): _prog(0)
{
}

Shader_prog::~Shader_prog()
{
    if(_prog)
        glDeleteProgram(_prog);
}

void Shader_prog::init(const std::vector<std::pair<std::string, GLenum>> & sources,
    const std::vector<std::pair<std::string, GLuint>> & attribs)
{
    std::vector<GLuint> shaders;
    for(const auto & source: sources)
    {
        // open shader file
        std::ifstream in(source.first, std::ios::binary | std::ios::in);
        std::vector <char> buff;

        if(in)
        {
            in.seekg(0, std::ios::end);
            size_t in_size = in.tellg();
            in.seekg(0, std::ios::beg);

            buff.resize(in_size + 1);
            buff.back() = '\0';
            in.read(buff.data(), in_size);

            if(!in)
            {
                for(auto & shader: shaders)
                    glDeleteShader(shader);
                throw std::ios_base::failure(std::string("Error reading shader file: ") + source.first);
            }
        }
        else
        {
            for(auto & shader: shaders)
                glDeleteShader(shader);
            throw std::ios_base::failure(std::string("Error opening shader file: ") + source.first);
        }

        // compile shaders
        shaders.push_back(glCreateShader(source.second));
        const char * source_str = buff.data();
        glShaderSource(shaders.back(), 1, &source_str, NULL);
        glCompileShader(shaders.back());

        // check for compilation errors
        GLint compile_status;
        glGetShaderiv(shaders.back(), GL_COMPILE_STATUS, &compile_status);

        if(compile_status != GL_TRUE)
        {
            GLint log_length;
            glGetShaderiv(shaders.back(), GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> log(log_length + 1);
            log.back() = '\0';
            glGetShaderInfoLog(shaders.back(), log_length, NULL, log.data());

            for(auto & shader: shaders)
                glDeleteShader(shader);

            throw std::system_error(compile_status, std::system_category(), std::string("Error compiling shader: ") +
                source.first + std::string("\n") + std::string(log.data()));
        }
    }

    _prog = glCreateProgram();

    for(auto & shader: shaders)
        glAttachShader(_prog, shader);

    // bind given attributes (must be done before link)
    for(auto & attr: attribs)
        glBindAttribLocation(_prog, attr.second, attr.first.c_str());

    glLinkProgram(_prog);

    for(auto & shader: shaders)
        glDeleteShader(shader);

    // check for link errors
    GLint link_status;
    glGetProgramiv(_prog, GL_LINK_STATUS, &link_status);
    if(link_status != GL_TRUE)
    {
        GLint log_length;
        glGetProgramiv(_prog, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetProgramInfoLog(_prog, log_length, NULL, log.data());

        glDeleteProgram(_prog);
        _prog = 0;

        throw std::system_error(link_status, std::system_category(), std::string("Error linking shader program:\n") +
            std::string(log.data()));
    }
}

bool Shader_prog::add_uniform(const std::string & uniform)
{
    GLint loc = glGetUniformLocation(_prog, uniform.c_str());
    uniforms[uniform] = loc;
    return loc != -1;
}

GLuint Shader_prog::operator()() const
{
    if(_prog)
        return _prog;
    else
        throw std::runtime_error(std::string("Attempt to use uninitialized shader prog"));
}

class Skybox // TODO: will probably inherit from some generic "renderable" class
{
public:
    Skybox();
    ~Skybox();
    void init();
    void draw(const Camera & cam, const glm::mat4 & proj);
protected:
    GLuint _vao, _vbo, _ebo;
    Texture_cubemap _tex;
    GLuint _num_indexes;
    Shader_prog _prog;
};

Skybox::Skybox(): _vao(0), _vbo(0), _ebo(0), _prog()
{
}

Skybox::~Skybox()
{
    if(_vao)
        glDeleteVertexArrays(1, &_vao);
    if(_vbo)
        glDeleteBuffers(1, &_vbo);
    if(_ebo)
        glDeleteBuffers(1, &_ebo);
}

void Skybox::init()
{
    // TODO: a sphere might look better
    std::vector<glm::vec3> vert_pos =
    {
        glm::vec3(-1.0f, -1.0f, -1.0f), // 0
        glm::vec3(1.0f, -1.0f, -1.0f), // 1
        glm::vec3(-1.0f, 1.0f, -1.0f), // 2
        glm::vec3(1.0f, 1.0f, -1.0f), // 3
        glm::vec3(-1.0f, -1.0f, 1.0f), // 4
        glm::vec3(1.0f, -1.0f, 1.0f), // 5
        glm::vec3(-1.0f, 1.0f, 1.0f), // 6
        glm::vec3(1.0f, 1.0f, 1.0f) // 7
    };

    std::vector<GLuint> index =
    {
        // front
        0, 1, 5,
        0, 5, 4,
        // right
        1, 3, 7,
        1, 7, 5,
        // back
        3, 2, 6,
        3, 6, 7,
        // left
        2, 0, 4,
        2, 4, 6,
        // top
        4, 5, 7,
        4, 7, 6,
        // bottom
        2, 3, 1,
        2, 1, 0
    };

    // create OpenGL vertex objects
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vert_pos.size(), vert_pos.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index.size(), index.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    _num_indexes = index.size();

    check_error("Skybox::init");

    _prog.init({std::make_pair("shaders/skybox.vert", GL_VERTEX_SHADER), std::make_pair("shaders/skybox.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)});
    _prog.add_uniform("model_view_proj");

    _tex.init("img/bluecloud_lf.jpg", "img/bluecloud_rt.jpg", "img/bluecloud_bk.jpg", "img/bluecloud_ft.jpg", "img/bluecloud_dn.jpg", "img/bluecloud_up.jpg");
}

void Skybox::draw(const Camera & cam, const glm::mat4 & proj)
{
    GLint depth_func_old;
    glGetIntegerv(GL_DEPTH_FUNC, &depth_func_old);

    glDepthFunc(GL_LEQUAL);

    glUseProgram(_prog()); // TODO: make this a use, or make the bind method below a operator(). be consistent
    _tex.bind();

    glm::mat4 model_view_proj = proj * glm::translate(cam.view_mat(), cam.pos());

    glUniformMatrix4fv(_prog.uniforms["model_view_proj"], 1, GL_FALSE, &model_view_proj[0][0]);

    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _num_indexes, GL_UNSIGNED_INT, (GLvoid *)0);

    glBindVertexArray(0); // get prev val?
    glUseProgram(0); // get prev val?

    glDepthFunc(depth_func_old);

    check_error("Skybox::draw");
}

class World
{
public:
    World(); // TODO should we take default args?
    bool init();
    void draw();
    void resize();
    void game_loop();
protected:
    void event_loop();
    void main_loop();

    bool _running;
    bool _focused;
    bool _do_resize; // TODO: find a better way (prob. messages/signals)

    sf::Mutex _lock; // TODO more descriptive name

    sf::Window _win;
    glm::mat4 _proj;
    Skybox _skybox;
    Player _player;
    std::vector<std::unique_ptr<Entity>> _entities;
};

World::World(): _running(true), _focused(true), _do_resize(false),
    _win(sf::VideoMode(800, 600), "mazerun", sf::Style::Default, sf::ContextSettings(24, 8, 8, 3, 0))
{
    std::cout<<"V: "<<_win.getSettings().majorVersion<<"."<<_win.getSettings().minorVersion<<std::endl;
    std::cout<<"D: "<<_win.getSettings().depthBits<<std::endl;
    std::cout<<"S: "<<_win.getSettings().stencilBits<<std::endl;
    std::cout<<"A: "<<_win.getSettings().antialiasingLevel<<std::endl;
}

bool World::init()
{
    if(glewInit() != GLEW_OK)
    {
        std::cerr<<"Error loading glew"<<std::endl;
        return false; // TODO: exception? at least be consistent w/ other inits
    }

    _win.setKeyRepeatEnabled(false);
    _win.setFramerateLimit(60);
    // TODO _win.setIcon

    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.0f, 1.0f);
    glLineWidth(5.0f);

    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendColor(1.0f, 1.0f, 1.0f, 0.1f);
    glEnable(GL_BLEND);

    resize();

    // set camera initial position / orientation
    _player.set(glm::vec3(1.0f, -10.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // set clear color
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    _skybox.init();
    // _player.init();
    return true;
}

void World::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _skybox.draw(_player, _proj);
    _win.display();
}

void World::resize()
{
    // projection matrix setup
    glViewport(0, 0, _win.getSize().x, _win.getSize().y);
    _proj = glm::perspective((float)M_PI / 6.0f,
        (float)_win.getSize().x / (float)_win.getSize().y, 0.1f, 1000.0f);
    // TODO: request redraw
}

void World::game_loop()
{
    // due to quirk of SFML event handling needs to be in main thread
    _win.setActive(false); // decativate rendering context for main thread

    // rendering, physics, input, etc to be handled in this thread
    sf::Thread main_loop_t(&World::main_loop, this);
    main_loop_t.launch();

    event_loop(); // handle events

    main_loop_t.wait();
    _win.close();
}

void World::event_loop()
{
    // TODO mutex
    while(true)
    {
        // handle events
        sf::Event ev;
        if(_win.waitEvent(ev)) // blocking call
        {
            _lock.lock();
            // TODO: have events trigger signals that listeners can recieve?
            switch(ev.type)
            {
                case sf::Event::Closed:
                    _running = false;
                    break;
                case sf::Event::GainedFocus:
                    _focused = true;
                    break;
                case sf::Event::LostFocus:
                    _focused = false;
                    break;
                case sf::Event::Resized:
                    resize();
                    _do_resize = true;
                    break;
                default:
                    break;
            }
        }
        if(!_running)
        {
            _lock.unlock();
            break;
        }
        _lock.unlock();
    }
}

// runs in a new thread
void World::main_loop()
{
    _win.setActive(true); // set render context active for this thread
    while(true)
    {
        _lock.lock();
        if(_do_resize)
        {
            resize();
            _do_resize = false;
        }

        if(!_running)
        {
            _lock.unlock();
            break;
        }
        // std::cerr<<"("<<_player.pos().x<<","<<_player.pos().y<<","<<_player.pos().z<<")"<<std::endl;
        if(_focused)
        {
            _player.handle_input(_win, 1.0f);
        }
        // TODO should we make more threads for input, physics, messages, etc?
        // TODO: physics / AI updates
        draw();
        _lock.unlock();
        // TODO sleep
    }
}

int main(int argc, char * argv[])
{
    #ifdef __linux
    XInitThreads(); // needed for multithreaded window access on Linux
    #endif

    // initialize glew
    World world;
    if(!world.init())
    {
        return EXIT_FAILURE;
    }

    world.game_loop();

    return EXIT_SUCCESS;
}
