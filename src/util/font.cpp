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

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>

#include <cerrno>
#include <cstring> // for std::strerror

#include "util/logger.hpp"

// TODO: documentation
Font_sys::Font_sys(const std::string & font_name, const unsigned int font_size,
    const unsigned int v_dpi, const unsigned int h_dpi)
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
    _cell_bbox.xMin = FT_MulFix(_face->bbox.xMin, _face->size->metrics.x_scale) / 64;
    _cell_bbox.yMin = FT_MulFix(_face->bbox.yMin, _face->size->metrics.y_scale) / 64;
    _cell_bbox.xMax = FT_MulFix(_face->bbox.xMax, _face->size->metrics.x_scale) / 64;
    _cell_bbox.yMax = FT_MulFix(_face->bbox.yMax, _face->size->metrics.y_scale) / 64;

    _tex_width = (_cell_bbox.xMax - _cell_bbox.xMin) * 16;
    _tex_height = (_cell_bbox.yMax - _cell_bbox.yMin) * 16;

    _has_kerning_info = FT_HAS_KERNING(_face);

    ++_lib_ref_cnt;
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

// TODO: replace w/ opengl calls
void Font_sys::render_text(const std::string & utf8_input, const std::string & filename)
{
    // libiconv has a weird API, and requires a non-const input
    render_text(std::string(utf8_input), filename);
}

void Font_sys::render_text(std::string & utf8_input, const std::string & filename)
{
    std::u32string utf32_str;

    FT_Vector pen = {0, 0};

    FT_BBox text_bounds;
    text_bounds.xMin = std::numeric_limits<int>::max();
    text_bounds.yMin = std::numeric_limits<int>::max();
    text_bounds.xMax = std::numeric_limits<int>::min();
    text_bounds.yMax = std::numeric_limits<int>::min();

    // TODO: do we really need to get the bound box?
    FT_UInt prev_glyph_i = 0;

    char * in = &utf8_input[0];
    std::size_t in_left = utf8_input.size();
    while(in_left > 0)
    {
        uint32_t code_pt;
        char * out = reinterpret_cast<char *>(&code_pt);
        std::size_t out_left = sizeof(uint32_t);

        _iconv_lib->convert(in, in_left, out, out_left);
        utf32_str.push_back(code_pt);

        auto page_i = _page_map.find(code_pt >> 8);
        if(page_i == _page_map.end())
        {
            page_i = load_page(code_pt >> 8);
        }

        Page & page = page_i->second;
        Char_info & c = page.char_info[code_pt & 0xFF];

        if(_has_kerning_info && prev_glyph_i && c.glyph_i)
        {
            FT_Vector kerning = {0, 0};
            if(FT_Get_Kerning(_face, prev_glyph_i, c.glyph_i, FT_KERNING_DEFAULT, &kerning) != FT_Err_Ok)
            {
                std::ostringstream ostream;
                ostream<<"Can't load kerning for: "<<std::hex<<std::showbase<<code_pt;
                Logger_locator::get()(Logger::WARN, ostream.str());
            }
            pen.x += kerning.x / 64;
            pen.y += kerning.y / 64;
        }

        text_bounds.xMin = std::min(text_bounds.xMin, pen.x + c.bbox.xMin);
        text_bounds.yMin = std::min(text_bounds.yMin, pen.y + c.bbox.yMin);
        text_bounds.xMax = std::max(text_bounds.xMax, pen.x + c.bbox.xMax);
        text_bounds.yMax = std::max(text_bounds.yMax, pen.y + c.bbox.yMax);

        pen.x += c.advance.x / 64;
        pen.y += c.advance.y / 64;

        prev_glyph_i = c.glyph_i;
    }

    std::size_t img_width = text_bounds.xMax - text_bounds.xMin;
    std::size_t img_height = text_bounds.yMax - text_bounds.yMin;
    std::vector<char> pixmap(img_height * img_width, 0);

    pen.x = -text_bounds.xMin;
    pen.y = text_bounds.yMax;

    prev_glyph_i = 0;

    for(const auto & code_pt: utf32_str)
    {
        auto page_i = _page_map.find(code_pt >> 8);
        Page & page = page_i->second;
        Char_info & c = page.char_info[code_pt & 0xFF];

        if(_has_kerning_info && prev_glyph_i && c.glyph_i)
        {
            FT_Vector kerning = {0, 0};
            if(FT_Get_Kerning(_face, prev_glyph_i, c.glyph_i, FT_KERNING_DEFAULT, &kerning) != FT_Err_Ok)
            {
                std::ostringstream ostream;
                ostream<<"Can't load kerning for: "<<std::hex<<std::showbase<<code_pt;
                Logger_locator::get()(Logger::WARN, ostream.str());
            }
            pen.x += kerning.x / 64;
            pen.y += kerning.y / 64;
        }

        for(int y = 0; y < c.bbox.yMax - c.bbox.yMin; ++y)
        {
            for(int x = 0; x < c.bbox.xMax - c.bbox.xMin; ++x)
            {
                long img_y = pen.y - c.bbox.yMax + y;
                long img_x = pen.x + c.bbox.xMin + x;

                unsigned short tbl_row = (code_pt >> 4) & 0xF;
                unsigned short tbl_col = code_pt & 0xF;

                long tbl_y = tbl_row * (_cell_bbox.yMax - _cell_bbox.yMin) + _cell_bbox.yMax - c.bbox.yMax + y;
                long tbl_x = tbl_col * (_cell_bbox.xMax - _cell_bbox.xMin) - _cell_bbox.xMin + c.bbox.xMin + x;

                pixmap[img_y * img_width + img_x] += page.tex[tbl_y * _tex_width + tbl_x];
            }
        }

        pen.x += c.advance.x / 64;
        pen.y += c.advance.y / 64;

        prev_glyph_i = c.glyph_i;
    }

    std::ofstream out_file(filename, std::ios_base::binary);
    out_file<<"P5\n"<<img_width<<" "<<img_height<<"\n"<<255<<"\n";
    for(size_t y = 0; y < img_height; ++y)
    {
        for(size_t x = 0; x < img_width; ++x)
        {
            out_file<<pixmap[y * img_width + x];
        }
    }
}

std::unordered_map<uint32_t, Font_sys::Page>::iterator Font_sys::load_page(const uint32_t page_no)
{
    std::ostringstream ostream;
    ostream<<"Loading font page "<<std::hex<<std::showbase<<page_no;
    Logger_locator::get()(Logger::TRACE, ostream.str());

    auto page_i = _page_map.emplace(std::make_pair(page_no, Page())).first;
    Page & page = page_i->second;

    page.tex.resize(_tex_width * _tex_height, 0); // TODO: luminance w/ alpha

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

        c.origin.x = -_cell_bbox.xMin + slot->bitmap_left;
        c.origin.y = _cell_bbox.yMax - slot->bitmap_top;
        c.bbox.xMin = slot->bitmap_left;
        c.bbox.xMax = (int)bmp->width + slot->bitmap_left;
        c.bbox.yMin = slot->bitmap_top - (int)bmp->rows;
        c.bbox.yMax = slot->bitmap_top;
        c.advance = slot->advance;
        c.glyph_i = glyph_i;

        for(std::size_t y = 0; y < (std::size_t)bmp->rows; ++y)
        {
            for(std::size_t x = 0; x < (std::size_t)bmp->width; ++x)
            {
                long tbl_img_y = tbl_row * (_cell_bbox.yMax - _cell_bbox.yMin) + _cell_bbox.yMax - slot->bitmap_top + y;
                long tbl_img_x = tbl_col * (_cell_bbox.xMax - _cell_bbox.xMin) - _cell_bbox.xMin + slot->bitmap_left + x;

                // TODO: monochrome fonts?
                page.tex[tbl_img_y * _tex_width + tbl_img_x] = bmp->buffer[y * bmp->width + x];
            }
        }
    }

    // TODO: remove
    /*
    std::ofstream out_file("page-" + std::to_string(page_no), std::ios_base::binary);
    out_file<<"P5\n"<<_tex_width<<" "<<_tex_height<<"\n"<<255<<"\n";
    for(size_t y = 0; y < _tex_height; ++y)
    {
        for(size_t x = 0; x < _tex_width; ++x)
        {
            out_file<<page.tex[y * _tex_width + x];
        }
    }
    */

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
    if((bytes_converted == (size_t)-1 && errno != 0) || (errno != 0 && errno != E2BIG))
    {
        std::system_error err;
        switch(errno)
        {
        case EILSEQ:
            Logger_locator::get()(Logger::WARN, "Illiegal char sequence in iconv conversion");
            err = std::system_error(errno, std::system_category(), "Illiegal char sequence in iconv conversion");
            break;
        case EINVAL:
            Logger_locator::get()(Logger::WARN, "Incomplete char sequence in iconv conversion");
            err = std::system_error(errno, std::system_category(), "Incomplete char sequence in iconv conversion");
            break;
        default:
            Logger_locator::get()(Logger::WARN, std::string("Unknown error in iconv conversion") + std::strerror(errno));
            err = std::system_error(errno, std::system_category(), std::string("Unknown error in iconv conversion") + std::strerror(errno));
            break;
        }
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
