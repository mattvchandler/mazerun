// font.hpp
// font loading and text display

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

#ifndef FONT_HPP
#define FONT_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector> // TODO: remove
#include <cstdint>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iconv.h>

#include <fontconfig/fontconfig.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

template<typename T>
struct Bbox
{
    #if(defined(GLM_PRECISION_LOWP_FLOAT))
    glm::tvec2<T, glm::lowp> ul;
    glm::tvec2<T, glm::lowp> lr;
    #elif(defined(GLM_PRECISION_MEDIUMP_FLOAT))
    glm::tvec2<T, glm::mediump> ul;
    glm::tvec2<T, glm::mediump> lr;
    #else
    glm::tvec2<T, glm::highp> ul;
    glm::tvec2<T, glm::highp> lr;
    #endif

    T width() const
    {
        return lr.x - ul.x;
    }
    T height() const
    {
        return ul.y - lr.y;
    }
};

class Font_sys
{
public:
    Font_sys(const std::string & font_name, const unsigned int font_size,
        const unsigned int v_dpi = 96, const unsigned int h_dpi = 96);
    ~Font_sys();

    // TODO: replace w/ opengl calls
    void render_text(const std::string & utf8_input, const std::string & filename);
    void render_text(std::string & utf8_input, const std::string & filename);

protected:
    class Freetype_lib
    {
    public:
        Freetype_lib();
        ~Freetype_lib();

        FT_Library & get_lib();
        const FT_Library & get_lib() const;

    protected:
        FT_Library _lib;
    };

    class Iconv_lib
    {
    public:
        Iconv_lib(const std::string & to_encoding, const std::string & from_encoding);
        ~Iconv_lib();

        std::size_t convert(char *& input, std::size_t & num_input_bytes,
            char *& output, std::size_t & num_output_bytes);

    protected:
        iconv_t _lib;
    };

    class Fontconfig_lib
    {
    public:
        Fontconfig_lib();
        ~Fontconfig_lib();

        FcConfig * get_config();
        const FcConfig * get_config() const;

    protected:
        FcConfig * _fc_config;
    };

    struct Char_info
    {
        glm::ivec2 origin;
        glm::ivec2 advance;
        Bbox<int> bbox;
        FT_UInt glyph_i;
    };

    struct Page
    {
        std::vector<char> tex; // TODO: replace w/ texture
        Char_info char_info[256];
    };

    std::unordered_map<uint32_t, Page>::iterator load_page(const uint32_t page_no);

    FT_Face _face;
    bool _has_kerning_info;

    Bbox<int> _cell_bbox;

    size_t _tex_width, _tex_height;

    std::unordered_map<uint32_t, Page> _page_map;

    static unsigned int _lib_ref_cnt;
    static std::unique_ptr<Freetype_lib> _ft_lib;
    static std::unique_ptr<Iconv_lib> _iconv_lib;
    static std::unique_ptr<Fontconfig_lib> _fontconfig_lib;
};

#endif // FONT_HPP
