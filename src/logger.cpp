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

std::shared_ptr<Logger> Logger_locator::_log;
std::shared_ptr<Logger> Logger_locator::_default_logger = std::make_shared<Null_log>();

void Logger_locator::init(std::shared_ptr<Logger> log)
{
    if(!log)
        _log = _default_logger;
    else
        _log = log;
}

Logger & Logger_locator::get()
{
    return *_log;
}
