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
#include <cstdint>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iconv.h>

#include <fontconfig/fontconfig.h>

#include "opengl/gl_wrappers.hpp"
#include "opengl/shader_prog.hpp"
#include "opengl/texture.hpp"

// container for font and text rendering
class Font_sys final
{
public:
    enum {ORIGIN_HORIZ_BASELINE = 0x00, ORIGIN_HORIZ_LEFT = 0x01, ORIGIN_HORIZ_RIGHT = 0x02, ORIGIN_HORIZ_CENTER = 0x03,
        ORIGIN_VERT_BASELINE = 0x00, ORIGIN_VERT_TOP = 0x04, ORIGIN_VERT_BOTTOM = 0x08, ORIGIN_VERT_CENTER = 0x0C};

    // Load font libraries and open a font file
    Font_sys(const std::string & font_name, const unsigned int font_size,
        const unsigned int v_dpi = 96, const unsigned int h_dpi = 96);
    // deallocate font
    ~Font_sys();

    // render text (rebuilds for each frame - use Static_text if text doesn't change)
    void render_text(const std::string & utf8_input, const glm::vec4 & color,
        const glm::vec2 & win_size, const glm::vec2 & pos, const int align_flags = 0);

protected:
    // RAII class for freetype library
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

    // RAII class for iconv library for a given encoding conversion
    class Iconv_lib
    {
    public:
        Iconv_lib(const std::string & to_encoding, const std::string & from_encoding);
        ~Iconv_lib();

        // iconv unicode encoding conversion
        // exits w/o error for E2BIG, allowing for char-by-char conversion
        std::size_t convert(char *& input, std::size_t & num_input_bytes,
            char *& output, std::size_t & num_output_bytes);

    protected:
        iconv_t _lib;
    };

    // RAII class for fontconfig library
    class Fontconfig_lib final
    {
    public:
        Fontconfig_lib();
        ~Fontconfig_lib();

        FcConfig * get_config();
        const FcConfig * get_config() const;

    protected:
        FcConfig * _fc_config;
    };

    // container for common libraries and shader program
    // every Font_sys obj can use the same instance of these
    struct Static_common
    {
        Static_common(const std::string & to_encoding, const std::string & from_encoding,
            const std::vector<std::pair<std::string, GLenum>> & sources,
            const std::vector<std::pair<std::string, GLuint>> & attribs,
            const std::vector<std::pair<std::string, GLuint>> & frag_data = {});

        Freetype_lib ft_lib;
        Iconv_lib iconv_lib;
        Fontconfig_lib fontconfig_lib;
        Shader_prog prog;
    };

    // common libraries and reference count
    static unsigned int _lib_ref_cnt;
    static std::unique_ptr<Static_common> _static_common;

    // bounding box
    template<typename T>
    struct Bbox
    {
        glm::tvec2<T> ul;
        glm::tvec2<T> lr;

        T width() const
        {
            return lr.x - ul.x;
        }
        T height() const
        {
            return ul.y - lr.y;
        }
    };

    // vertex buffer coordinate data
    struct Coord_data
    {
        uint32_t page_no;
        std::size_t start;
        std::size_t num_elements;
    };

    // character info
    struct Char_info
    {
        glm::ivec2 origin;
        glm::ivec2 advance;
        Bbox<int> bbox;
        FT_UInt glyph_i;
    };

    // font page
    struct Page
    {
        std::unique_ptr<Texture_2D> tex;
        Char_info char_info[256];
    };

    // create a font page texture
    std::unordered_map<uint32_t, Page>::iterator load_page(const uint32_t page_no);

    // font data
    FT_Face _face;
    bool _has_kerning_info;
    Bbox<int> _cell_bbox;
    int _line_height;

    // texture size
    size_t _tex_width, _tex_height;

    // font pages
    std::unordered_map<uint32_t, Page> _page_map;

    // OpenGL vertex object
    GL_vertex_array _vao;
    GL_buffer _vbo;

    friend class Static_text;
    friend std::pair<std::vector<glm::vec2>, std::vector<Font_sys::Coord_data>>
        build_text(const std::string & utf8_input, Font_sys & font_sys,
        Font_sys::Bbox<float> & font_box_out);

};

// object for text which does not change often
class Static_text final
{
public:
    // create and build text buffer object
    Static_text(Font_sys & font, const std::string & utf8_input, const glm::vec4 & color);
    // recreate text object with new string
    void set_text(Font_sys & font, const std::string & utf8_input);
    // set font color
    void set_color(const glm::vec4 & color);
    // render the text
    void render_text(Font_sys & font, const glm::vec2 & win_size,
        const glm::vec2 & pos, const int align_flags = 0);

protected:
    GL_vertex_array _vao;
    GL_buffer _vbo;
    glm::vec4 _color;
    std::vector<Font_sys::Coord_data> _coord_data;
    Font_sys::Bbox<float> _text_box;
};

#endif // FONT_HPP
