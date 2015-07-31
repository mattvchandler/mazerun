// font_libs.cpp
// shared font libraries

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

#include "util/font.hpp"

#include <stdexcept>
#include <system_error>

#include <cerrno>
#include <cstring> // for std::strerror

#include "util/logger.hpp"

// RAII class for freetype library
Font_sys::Freetype_lib::Freetype_lib()
{
    Logger_locator::get()(Logger::DBG, "Loading freetype library");
    FT_Error err = FT_Init_FreeType(&_lib);
    if(err != FT_Err_Ok)
    {
        Logger_locator::get()(Logger::ERROR, "Error loading freetype library");
        throw std::system_error(err, std::system_category(), "Error loading freetype library");
    }
}

Font_sys::Freetype_lib::~Freetype_lib()
{
    Logger_locator::get()(Logger::DBG, "Unloading freetype library");
    FT_Done_FreeType(_lib);
}

FT_Library & Font_sys::Freetype_lib::get_lib()
{
    return _lib;
}

const FT_Library & Font_sys::Freetype_lib::get_lib() const
{
    return _lib;
}

// RAII class for iconv library for a given encoding conversion
Font_sys::Iconv_lib::Iconv_lib(const std::string & to_encoding, const std::string & from_encoding)
{
    Logger_locator::get()(Logger::DBG, "Loading libiconv");
    errno = 0;
    _lib = iconv_open(to_encoding.c_str(), from_encoding.c_str());
    if(errno != 0)
    {
        if(errno == EINVAL)
        {
            Logger_locator::get()(Logger::ERROR, "Error loading libiconv: can't convert " + from_encoding + " to " + to_encoding);
            auto err = std::system_error(errno, std::system_category(), "Error loading libiconv: can't convert " + from_encoding + " to " + to_encoding);
            errno = 0;
            throw err;
        }
        else
        {
            Logger_locator::get()(Logger::ERROR, std::string("Error loading libiconv: ") + std::strerror(errno));
            auto err = std::system_error(errno, std::system_category(), std::string("Error loading libiconv: ") + std::strerror(errno));
            errno = 0;
            throw err;
        }
    }
}

Font_sys::Iconv_lib::~Iconv_lib()
{
    Logger_locator::get()(Logger::DBG, "Unloading libiconv");
    iconv_close(_lib);
}

// iconv unicode encoding conversion
// exits w/o error for E2BIG, allowing for char-by-char conversion
std::size_t Font_sys::Iconv_lib::convert(char *& input, std::size_t & num_input_bytes,
    char *& output, std::size_t & num_output_bytes)
{
    errno = 0;
    std::size_t bytes_converted = iconv(_lib, &input, &num_input_bytes, &output, &num_output_bytes);
    if((errno != 0 && errno != E2BIG))
    {
        std::system_error err;
        switch(errno)
        {
        case EILSEQ:
            Logger_locator::get()(Logger::WARN, "Illegal char sequence in iconv conversion: ");
            err = std::system_error(errno, std::system_category(), "Illiegal char sequence in iconv conversion: ");
            break;
        case EINVAL:
            Logger_locator::get()(Logger::WARN, "Incomplete char sequence in iconv conversion: ");
            err = std::system_error(errno, std::system_category(), "Incomplete char sequence in iconv conversion: ");
            break;
        default:
            Logger_locator::get()(Logger::WARN, std::string("Unknown error in iconv conversion: ") + std::strerror(errno));
            err = std::system_error(errno, std::system_category(), std::string("Unknown error in iconv conversion: ") + std::strerror(errno));
            break;
        }
        throw err;
    }
    return bytes_converted;
}

// RAII class for fontconfig library
Font_sys::Fontconfig_lib::Fontconfig_lib()
{
    Logger_locator::get()(Logger::DBG, "Loading fontconfig");
    if(!FcInit())
    {
        Logger_locator::get()(Logger::ERROR, "Error loading fontconfig");
        throw std::runtime_error("Error loading freetype library");
    }
    _fc_config = FcInitLoadConfigAndFonts();
}

Font_sys::Fontconfig_lib::~Fontconfig_lib()
{
    Logger_locator::get()(Logger::DBG, "Unloading fontconfig");
    FcConfigDestroy(_fc_config);
    FcFini();
}

FcConfig * Font_sys::Fontconfig_lib::get_config()
{
    return _fc_config;
}

const FcConfig * Font_sys::Fontconfig_lib::get_config() const
{
    return _fc_config;
}

Font_sys::Static_common::Static_common(const std::string & to_encoding, const std::string & from_encoding,
    const std::vector<std::pair<std::string, GLenum>> & sources,
    const std::vector<std::pair<std::string, GLuint>> & attribs,
    const std::vector<std::pair<std::string, GLuint>> & frag_data):
    iconv_lib(to_encoding, from_encoding),
    prog(sources, attribs, frag_data)
{}

unsigned int Font_sys::_lib_ref_cnt = 0;
std::unique_ptr<Font_sys::Static_common> Font_sys::_static_common;
