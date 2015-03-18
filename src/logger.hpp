// logger.hpp
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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

class Logger
{
public:
    typedef enum {TRACE, DBG, INFO, WARN, ERROR} Level;

    Logger(const Level lvl = INFO);
    virtual ~Logger() = default;
    virtual void operator()(const Level lvl, const std::string & msg);
    void set_level(const Level lvl);
    Level get_level();

protected:
    std::string level_to_str(const Level lvl);
    std::string timestamp(unsigned int dec_places = 4);
    std::string preamble(const Level lvl);

    std::mutex _lock;
    Level _lvl;
};

class Tee_log final: public Logger
{
public:
    Tee_log(const std::string & filename, std::ostream & stream = std::cerr, const Level lvl = INFO);
    void operator()(const Level lvl, const std::string & msg);

private:
    std::ofstream _file;
    std::ostream & _stream;
};

class Logger_locator
{
public:
    Logger_locator() = delete;
    ~Logger_locator() = delete;
    static void init(std::shared_ptr<Logger> log = _default_logger);
    static Logger & get();

private:
    static std::shared_ptr<Logger> _log;
    static std::shared_ptr<Logger> _default_logger;
};

#endif // LOGGER_HPP
