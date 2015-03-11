// logger.cpp
// logging and error-reporting

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

#include "logger.hpp"

#include <chrono>
#include <iomanip>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>

// check for OpenGL error and print message
void check_error(const std::string & at)
{
    // TODO: disable checks in release mode
    GLenum e = glGetError();
    if(e == GL_NO_ERROR)
        return;

    Logger_locator::get()(Logger::WARN,
        "OpenGL Error at " + at + ": " + (const char *)(gluErrorString(e)));
}

std::ostream & operator<<(std::ostream & out, const glm::vec2 & v)
{
    return out<<"("<<v.x<<","<<v.y<<")";
}

std::ostream & operator<<(std::ostream & out, const glm::vec3 & v)
{
    return out<<"("<<v.x<<","<<v.y<<","<<v.z<<")";
}

std::ostream & operator<<(std::ostream & out, const glm::vec4 & v)
{
    return out<<"("<<v.x<<","<<v.y<<","<<v.z<<","<<v.w<<")";
}

std::ostream & operator<<(std::ostream & out, const glm::mat3 & m)
{
    out<<"[["<<m[0].x<<","<<m[1].x<<","<<m[2].x<<"\n";
    out<<" ["<<m[0].y<<","<<m[1].y<<","<<m[2].y<<"\n";
    return out<<" ["<<m[0].z<<","<<m[1].z<<","<<m[2].z<<"]]";
}

std::ostream & operator<<(std::ostream & out, const glm::mat4 & m)
{
    out<<"[["<<m[0].x<<","<<m[1].x<<","<<m[2].x<<","<<m[3].x<<"]\n";
    out<<" ["<<m[0].y<<","<<m[1].y<<","<<m[2].y<<","<<m[3].y<<"]\n";
    out<<" ["<<m[0].z<<","<<m[1].z<<","<<m[2].z<<","<<m[3].z<<"]\n";
    return out<<" ["<<m[0].w<<","<<m[1].w<<","<<m[2].w<<","<<m[3].w<<"]]";
}

std::ostream & operator<<(std::ostream & out, const glm::quat & q)
{
    return out<<"("<<q.x<<","<<q.y<<","<<q.z<<","<<q.w<<")";
}

Logger::Logger(const Level lvl): _lvl(lvl)
{
}

void Logger::operator()(const Level lvl, const std::string & msg)
{
    if(lvl >= _lvl)
    {
        std::cerr<<preamble(lvl)<<" "<<msg<<std::endl;
    }
}

void Logger::set_level(const Level lvl)
{
    _lvl = lvl;
}

Logger::Level Logger::get_level()
{
    return _lvl;
}

std::string Logger::level_to_str(const Level lvl)
{
    switch(lvl)
    {
    case TRACE:
        return "TRACE";
    case DBG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARN:
        return "WARN";
    case ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::timestamp(unsigned int dec_places)
{
    std::time_t now = std::time(nullptr);
    char buff[9] = {0};

    if(dec_places == 0)
    {
        std::strftime(buff, sizeof(buff), "%T", std::localtime(&now));
        return std::string(buff);
    }

    std::strftime(buff, sizeof(buff), "%H:%M", std::localtime(&now));

    auto now_chrono = std::chrono::system_clock::now().time_since_epoch();
    auto now_s = std::chrono::duration_cast<std::chrono::duration<double>>(now_chrono);
    auto now_m = std::chrono::duration_cast<std::chrono::minutes>(now_chrono);

    now_s -= now_m;

    std::ostringstream str;
    str<<buff<<":"<<std::fixed<<std::setprecision(dec_places)<<std::setw(dec_places + 3)<<std::setfill('0')<<now_s.count();
    return str.str();
}

std::string Logger::preamble(const Level lvl)
{
    return "[" + level_to_str(lvl) + "][" + timestamp() + "]";
}

Tee_log::Tee_log(const std::string & filename, std::ostream & stream, const Level lvl):
    Logger(lvl), _file(filename), _stream(stream)
{
}

void Tee_log::operator()(const Level lvl, const std::string & msg)
{
    std::string preamble_str = preamble(lvl);

    if(lvl >= _lvl)
    {
        _stream<<preamble_str<<" "<<msg<<std::endl;
        _file<<preamble_str<<" "<<msg<<std::endl;
    }
}

Logger * Logger_locator::_log;
Null_log Logger_locator::_default_logger;

void Logger_locator::init(Logger * log)
{
    if(!log)
        _log = &_default_logger;
    else
        _log = log;
}

Logger & Logger_locator::get()
{
    return *_log;
}
