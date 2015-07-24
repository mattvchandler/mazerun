// font.cpp
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

#include "util/font.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>

#include <cerrno>
#include <cstring> // for std::strerror

#include "opengl/gl_helpers.hpp"
#include "util/logger.hpp"

std::pair<std::vector<glm::vec2>, std::vector<Coord_data>> build_text(const std::string & utf8_input, const glm::vec2 & start,
    Font_sys & font_sys)
{
    glm::vec2 pen = start;

    FT_UInt prev_glyph_i = 0;

    std::unordered_map<uint32_t, std::vector<glm::vec2>> screen_and_tex_coords;

    char * in = const_cast<char *>(&utf8_input[0]);
    std::size_t in_left = utf8_input.size();
    while(in_left > 0)
    {
        uint32_t code_pt;
        char * out = reinterpret_cast<char *>(&code_pt);
        std::size_t out_left = sizeof(uint32_t);

        font_sys._iconv_lib->convert(in, in_left, out, out_left);

        uint32_t page_no = code_pt >> 8;
        auto page_i = font_sys._page_map.find(page_no);
        if(page_i == font_sys._page_map.end())
        {
            page_i = font_sys.load_page(code_pt >> 8);
        }

        Font_sys::Page & page = page_i->second;
        Font_sys::Char_info & c = page.char_info[code_pt & 0xFF];

        if(font_sys._has_kerning_info && prev_glyph_i && c.glyph_i)
        {
            FT_Vector kerning = {0, 0};
            if(FT_Get_Kerning(font_sys._face, prev_glyph_i, c.glyph_i, FT_KERNING_DEFAULT, &kerning) != FT_Err_Ok)
            {
                std::ostringstream ostream;
                ostream<<"Can't load kerning for: "<<std::hex<<std::showbase<<code_pt;
                Logger_locator::get()(Logger::WARN, ostream.str());
            }
            pen.x += kerning.x / 64;
            pen.y += kerning.y / 64;
        }

        std::size_t tex_row = (code_pt >> 4) & 0xF;
        std::size_t tex_col = code_pt & 0xF;

        glm::vec2 tex_origin = {(float)(tex_col * font_sys._cell_bbox.width() - font_sys._cell_bbox.ul.x),
            (float)(tex_row * font_sys._cell_bbox.height() + font_sys._cell_bbox.ul.y)};

        // TODO: screen coords or % or pixel coords?
        screen_and_tex_coords[page_no].push_back({(pen.x + c.bbox.ul.x) / 400.0f - 1.0f,
            1.0f - (pen.y - c.bbox.lr.y) / 300.0f});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.ul.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.lr.y) / font_sys._tex_height});
        screen_and_tex_coords[page_no].push_back({(pen.x + c.bbox.lr.x) / 400.0f - 1.0f,
            1.0f - (pen.y - c.bbox.lr.y) / 300.0f});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.lr.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.lr.y) / font_sys._tex_height});
        screen_and_tex_coords[page_no].push_back({(pen.x + c.bbox.ul.x) / 400.0f - 1.0f,
            1.0f - (pen.y - c.bbox.ul.y) / 300.0f});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.ul.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.ul.y) / font_sys._tex_height});

        screen_and_tex_coords[page_no].push_back({(pen.x + c.bbox.ul.x) / 400.0f - 1.0f,
            1.0f - (pen.y - c.bbox.ul.y) / 300.0f});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.ul.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.ul.y) / font_sys._tex_height});
        screen_and_tex_coords[page_no].push_back({(pen.x + c.bbox.lr.x) / 400.0f - 1.0f,
            1.0f - (pen.y - c.bbox.lr.y) / 300.0f});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.lr.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.lr.y) / font_sys._tex_height});
        screen_and_tex_coords[page_no].push_back({(pen.x + c.bbox.lr.x) / 400.0f - 1.0f,
            1.0f - (pen.y - c.bbox.ul.y) / 300.0f});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.lr.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.ul.y) / font_sys._tex_height});

        pen += c.advance / 64;

        prev_glyph_i = c.glyph_i;
    }

    std::pair<std::vector<glm::vec2>, std::vector<Coord_data>> coord_data;

    for(const auto & page: screen_and_tex_coords)
    {
        coord_data.second.emplace_back();
        Coord_data & c = coord_data.second.back();

        c.page_no = page.first;

        c.start = coord_data.first.size() / 2;
        coord_data.first.insert(coord_data.first.end(), page.second.begin(), page.second.end());
        c.num_elements = coord_data.first.size() / 2 - c.start;
    }
    return coord_data;
}

// TODO: documentation
Font_sys::Font_sys(const std::string & font_name, const unsigned int font_size,
    const unsigned int v_dpi, const unsigned int h_dpi):
    _vbo(GL_ARRAY_BUFFER),
    _prog({std::make_pair("shaders/text.vert", GL_VERTEX_SHADER),
        std::make_pair("shaders/text.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0), std::make_pair("vert_tex_coords", 1)})
{
    if(_lib_ref_cnt == 0)
    {
        _ft_lib.reset(new Freetype_lib);

        try
        {
            _iconv_lib.reset(new Iconv_lib("UTF-32LE", "UTF-8"));
        }
        catch(std::exception & e)
        {
            _ft_lib.reset();
            throw;
        }
        try
        {
            _fontconfig_lib.reset(new Fontconfig_lib);
        }
        catch(std::exception & e)
        {
            _ft_lib.reset();
            _iconv_lib.reset();
            throw;
        }
    }

    FcPattern * font_pat = FcNameParse(reinterpret_cast<const FcChar8 *>(font_name.c_str()));
    FcConfigSubstitute(_fontconfig_lib->get_config(), font_pat, FcMatchPattern);
    FcDefaultSubstitute(font_pat);

    FcResult result;
    FcPattern * matched_font_pat = FcFontMatch(_fontconfig_lib->get_config(), font_pat, &result);
    FcPatternDestroy(font_pat);

    std::string font_file;
    if(matched_font_pat)
    {
        FcChar8 * font_path_c;
        FcPatternGetString(matched_font_pat, FC_FILE, 0, &font_path_c);

        font_file = reinterpret_cast<char *>(font_path_c);

        Logger_locator::get()(Logger::DBG, "Found font file: " + font_file + " matching: " + font_name);

        FcPatternDestroy(matched_font_pat);
    }
    else
    {
        _ft_lib.reset();
        _iconv_lib.reset();
        _fontconfig_lib.reset();

        Logger_locator::get()(Logger::DBG, "Could not find font file matching: " + font_name);
        throw std::runtime_error("Could not find font file matching: " + font_name);
    }
    Logger_locator::get()(Logger::DBG, "Loading font file: " + font_file + " size: " + std::to_string(font_size));

    FT_Error err = FT_New_Face(_ft_lib->get_lib(), font_file.c_str(), 0, &_face);

    if(err != FT_Err_Ok)
    {
        if(_lib_ref_cnt == 0)
        {
            _ft_lib.reset();
            _iconv_lib.reset();
            _fontconfig_lib.reset();
        }
        if(err == FT_Err_Unknown_File_Format)
        {
            Logger_locator::get()(Logger::ERROR, "Unknown format for font file: " + font_file);
            throw std::system_error(err, std::system_category(), "Unknown format for font file: " + font_file);
        }
        else
        {
            Logger_locator::get()(Logger::ERROR, "Error reading font file: " + font_file);
            throw std::ios_base::failure("Error reading font file: " + font_file);
        }
    }

    if(FT_Select_Charmap(_face, FT_ENCODING_UNICODE) != FT_Err_Ok)
    {
        FT_Done_Face(_face);

        if(_lib_ref_cnt == 0)
        {
            _ft_lib.reset();
            _iconv_lib.reset();
            _fontconfig_lib.reset();
        }
        Logger_locator::get()(Logger::ERROR, "No unicode charmap in font file: " + font_file);
        throw std::system_error(err, std::system_category(), "No unicode charmap in font file: " + font_file);
    }

    if(FT_Set_Char_Size(_face, font_size * 64, font_size * 64, h_dpi, v_dpi) != FT_Err_Ok)
    {
        std::cerr<<"error setting size"<<std::endl;

        FT_Done_Face(_face);

        if(_lib_ref_cnt == 0)
        {
            _ft_lib.reset();
            _iconv_lib.reset();
            _fontconfig_lib.reset();
        }
        Logger_locator::get()(Logger::ERROR, "Can't set font size: " + std::to_string(font_size) + " for font file: " + font_file);
        throw std::system_error(err, std::system_category(), "Can't set font size: " + std::to_string(font_size) + " for font file: " + font_file);
    }

    // TODO: padding
    _cell_bbox.ul.x = FT_MulFix(_face->bbox.xMin, _face->size->metrics.x_scale) / 64 - 2;
    _cell_bbox.ul.y = FT_MulFix(_face->bbox.yMax, _face->size->metrics.y_scale) / 64 + 2;
    _cell_bbox.lr.x = FT_MulFix(_face->bbox.xMax, _face->size->metrics.x_scale) / 64 + 2;
    _cell_bbox.lr.y = FT_MulFix(_face->bbox.yMin, _face->size->metrics.y_scale) / 64 - 2;

    _tex_width = _cell_bbox.width() * 16;
    _tex_height = _cell_bbox.height() * 16;

    _has_kerning_info = FT_HAS_KERNING(_face);

    ++_lib_ref_cnt;

    _vao.bind();
    _vbo.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (const GLvoid *)sizeof(glm::vec2));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    _prog.use();
    _prog.add_uniform("font_page");
    _prog.add_uniform("color");
    glUniform1i(_prog.get_uniform("font_page"), 0);
    glUseProgram(0);

    check_error("Font_sys::Font_sys");
}

Font_sys::~Font_sys()
{
    FT_Done_Face(_face);

    if(--_lib_ref_cnt == 0)
    {
        _ft_lib.reset();
        _iconv_lib.reset();
        _fontconfig_lib.reset();
    }
    Logger_locator::get()(Logger::DBG, "Unloading font");
}

void Font_sys::render_text(const std::string & utf8_input, const glm::vec4 & color,
    const glm::vec2 & start)
{
    auto coord_data = build_text(utf8_input, start, *this);

    _vao.bind();
    _vbo.bind();

    glBufferData(_vbo.type(), sizeof(glm::vec2) * coord_data.first.size(), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(_vbo.type(), 0, sizeof(glm::vec2) * coord_data.first.size(), coord_data.first.data());

    _prog.use();
    glUniform4fv(_prog.get_uniform("color"), 1, &color[0]);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);

    for(const auto & cd: coord_data.second)
    {
        _page_map[cd.page_no].tex->bind();
        glDrawArrays(GL_TRIANGLES, cd.start, cd.num_elements);
    }

    glBindVertexArray(0);
    check_error("Font_sys::render_text");
}

std::unordered_map<uint32_t, Font_sys::Page>::iterator Font_sys::load_page(const uint32_t page_no)
{
    std::ostringstream ostream;
    ostream<<"Loading font page "<<std::hex<<std::showbase<<page_no;
    Logger_locator::get()(Logger::TRACE, ostream.str());

    auto page_i = _page_map.emplace(std::make_pair(page_no, Page())).first;
    Page & page = page_i->second;

    std::vector<char> tex_data(_tex_width * _tex_height, 0);

    FT_GlyphSlot slot = _face->glyph;

    for(uint32_t code_pt = page_no << 8; code_pt < ((page_no + 1) << 8); code_pt++)
    {
        unsigned short tbl_row = (code_pt >> 4) & 0xF;
        unsigned short tbl_col = code_pt & 0xF;

        FT_UInt glyph_i = FT_Get_Char_Index(_face, code_pt);
        if(FT_Load_Glyph(_face, glyph_i, FT_LOAD_RENDER) != FT_Err_Ok)
        {
            ostream.str("");
            ostream<<"Err loading glyph for: "<<std::hex<<std::showbase<<code_pt;
            Logger_locator::get()(Logger::WARN, ostream.str());
            continue;
        }

        FT_Bitmap * bmp = &slot->bitmap;
        Char_info & c = page.char_info[code_pt & 0xFF];

        c.origin.x = -_cell_bbox.ul.x + slot->bitmap_left;
        c.origin.y = _cell_bbox.ul.y - slot->bitmap_top;
        c.bbox.ul.x = slot->bitmap_left;
        c.bbox.ul.y = slot->bitmap_top;
        c.bbox.lr.x = (int)bmp->width + slot->bitmap_left;
        c.bbox.lr.y = slot->bitmap_top - (int)bmp->rows;
        c.advance.x = slot->advance.x;
        c.advance.y = slot->advance.y;
        c.glyph_i = glyph_i;

        for(std::size_t y = 0; y < (std::size_t)bmp->rows; ++y)
        {
            for(std::size_t x = 0; x < (std::size_t)bmp->width; ++x)
            {
                long tbl_img_y = tbl_row * _cell_bbox.height() + _cell_bbox.ul.y - slot->bitmap_top + y;
                long tbl_img_x = tbl_col * _cell_bbox.width() - _cell_bbox.ul.x + slot->bitmap_left + x;

                // TODO: monochrome fonts?
                tex_data[tbl_img_y * _tex_width + tbl_img_x] = bmp->buffer[y * bmp->width + x];
            }
        }
    }

    page.tex.reset(new Texture_2D);
    page.tex->bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _tex_width, _tex_height,
        0, GL_RED, GL_UNSIGNED_BYTE, tex_data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // set params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return page_i;
}

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
            Logger_locator::get()(Logger::WARN, "Illiegal char sequence in iconv conversion: ");
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

Font_sys::Fontconfig_lib::Fontconfig_lib()
{
    Logger_locator::get()(Logger::DBG, "Loading fontconfig");
    if(!FcInit())
    {
        Logger_locator::get()(Logger::ERROR, "Error loading fontcondig");
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

unsigned int Font_sys::_lib_ref_cnt = 0;
std::unique_ptr<Font_sys::Freetype_lib> Font_sys::_ft_lib;
std::unique_ptr<Font_sys::Iconv_lib> Font_sys::_iconv_lib;
std::unique_ptr<Font_sys::Fontconfig_lib> Font_sys::_fontconfig_lib;

Static_text::Static_text(Font_sys & font, const std::string & utf8_input,
    const glm::vec4 & color, const glm::vec2 & start):
    _vbo(GL_ARRAY_BUFFER),
    _color(color)
{
    Logger_locator::get()(Logger::DBG, "Creating static text");
    auto coord_data = build_text(utf8_input, start, font);

    _coord_data = coord_data.second;

    _vao.bind();
    _vbo.bind();
    glBufferData(_vbo.type(), sizeof(glm::vec2) * coord_data.first.size(),
        coord_data.first.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (const GLvoid *)sizeof(glm::vec2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    check_error("Static_text::Static_text");
}

// TODO: make static libs struct, add shader prog to it
void Static_text::render_text(Font_sys & font)
{
    font._prog.use();
    glUniform4fv(font._prog.get_uniform("color"), 1, &_color[0]);

    _vao.bind();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);

    for(const auto & cd: _coord_data)
    {
        font._page_map[cd.page_no].tex->bind();
        glDrawArrays(GL_TRIANGLES, cd.start, cd.num_elements);
    }

    glBindVertexArray(0);
    check_error("Static_text::render_text");
}
