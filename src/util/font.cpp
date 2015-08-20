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
#include <limits>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>

#include "opengl/gl_helpers.hpp"
#include "util/logger.hpp"

// Build buffer of quads for text display
std::pair<std::vector<glm::vec2>, std::vector<Font_sys::Coord_data>> build_text(const std::string & utf8_input,
    Font_sys & font_sys, Font_sys::Bbox<float> & font_box_out)
{
    glm::vec2 pen(0.0f, 0.0f);

    // verts by font page
    std::unordered_map<uint32_t, std::vector<glm::vec2>> screen_and_tex_coords;

    font_box_out.ul.x = std::numeric_limits<float>::max();
    font_box_out.ul.y = std::numeric_limits<float>::max();
    font_box_out.lr.x = std::numeric_limits<float>::min();
    font_box_out.lr.y = std::numeric_limits<float>::min();

    FT_UInt prev_glyph_i = 0;

    // cast input to non-const for iconv
    char * in = const_cast<char *>(&utf8_input[0]);
    std::size_t in_left = utf8_input.size();
    while(in_left > 0)
    {
        // handle newlines
        if(*in == '\n')
        {
            pen.x = 0;
            pen.y += font_sys._line_height;
            prev_glyph_i = 0;
            ++in;
            --in_left;
            continue;
        }

        // extract next unicode code pt
        uint32_t code_pt;
        char * out = reinterpret_cast<char *>(&code_pt);
        std::size_t out_left = sizeof(uint32_t);

        font_sys._static_common->iconv_lib.convert(in, in_left, out, out_left);

        // get font page struct
        uint32_t page_no = code_pt >> 8;
        auto page_i = font_sys._page_map.find(page_no);

        // load page if not already loaded
        if(page_i == font_sys._page_map.end())
        {
            page_i = font_sys.load_page(code_pt >> 8);
        }

        Font_sys::Page & page = page_i->second;
        Font_sys::Char_info & c = page.char_info[code_pt & 0xFF];

        // add kerning if necessary
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
            pen.y -= kerning.y / 64;
        }

        std::size_t tex_row = (code_pt >> 4) & 0xF;
        std::size_t tex_col = code_pt & 0xF;

        // texture coord of glyph's origin
        glm::vec2 tex_origin = {(float)(tex_col * font_sys._cell_bbox.width() - font_sys._cell_bbox.ul.x),
            (float)(tex_row * font_sys._cell_bbox.height() + font_sys._cell_bbox.ul.y)};

        // push back vertex coords, and texture coords, interleaved, into a map by font page
        // 1 unit to pixel scale
        // lower left corner
        screen_and_tex_coords[page_no].push_back({pen.x + c.bbox.ul.x,
            pen.y - c.bbox.lr.y});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.ul.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.lr.y) / font_sys._tex_height});
        // lower right corner
        screen_and_tex_coords[page_no].push_back({pen.x + c.bbox.lr.x,
            pen.y - c.bbox.lr.y});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.lr.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.lr.y) / font_sys._tex_height});
        // upper left corner
        screen_and_tex_coords[page_no].push_back({pen.x + c.bbox.ul.x,
            pen.y - c.bbox.ul.y});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.ul.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.ul.y) / font_sys._tex_height});

        // upper left corner
        screen_and_tex_coords[page_no].push_back({pen.x + c.bbox.ul.x,
            pen.y - c.bbox.ul.y});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.ul.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.ul.y) / font_sys._tex_height});
        // lower right corner
        screen_and_tex_coords[page_no].push_back({pen.x + c.bbox.lr.x,
            pen.y - c.bbox.lr.y});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.lr.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.lr.y) / font_sys._tex_height});
        // upper right corner
        screen_and_tex_coords[page_no].push_back({pen.x + c.bbox.lr.x,
            pen.y - c.bbox.ul.y});
        screen_and_tex_coords[page_no].push_back({(tex_origin.x + c.bbox.lr.x) / font_sys._tex_width,
            (tex_origin.y - c.bbox.ul.y) / font_sys._tex_height});

        // expand bounding box for whole string
        font_box_out.ul.x = std::min(font_box_out.ul.x, pen.x + c.bbox.ul.x);
        font_box_out.ul.y = std::min(font_box_out.ul.y, pen.y - c.bbox.ul.y);
        font_box_out.lr.x = std::max(font_box_out.lr.x, pen.x + c.bbox.lr.x);
        font_box_out.lr.y = std::max(font_box_out.lr.y, pen.y - c.bbox.lr.y);

        // advance to next origin
        pen.x += c.advance.x / 64;
        pen.y -= c.advance.y / 64;

        prev_glyph_i = c.glyph_i;
    }

    // reorganize texture data into a contiguous array
    std::pair<std::vector<glm::vec2>, std::vector<Font_sys::Coord_data>> coord_data;

    for(const auto & page: screen_and_tex_coords)
    {
        coord_data.second.emplace_back();
        Font_sys::Coord_data & c = coord_data.second.back();

        c.page_no = page.first;

        c.start = coord_data.first.size() / 2;
        coord_data.first.insert(coord_data.first.end(), page.second.begin(), page.second.end());
        c.num_elements = coord_data.first.size() / 2 - c.start;
    }

    return coord_data;
}

// Load font libraries and open a font file
Font_sys::Font_sys(const std::string & font_name, const unsigned int font_size,
    const unsigned int v_dpi, const unsigned int h_dpi):
    _vbo(GL_ARRAY_BUFFER)
{
    // load freetype, iconv, and fontconfig libraries, and text shader - only once
    if(_lib_ref_cnt == 0)
    {
        _static_common.reset(new Static_common("UTF-32LE", "UTF-8",
            {std::make_pair("shaders/text.vert", GL_VERTEX_SHADER),
            std::make_pair("shaders/text.frag", GL_FRAGMENT_SHADER)},
            {std::make_pair("vert_pos", 0), std::make_pair("vert_tex_coords", 1)}));
    }

    // find file for given font
    FcPattern * font_pat = FcNameParse(reinterpret_cast<const FcChar8 *>(font_name.c_str()));
    FcConfigSubstitute(_static_common->fontconfig_lib.get_config(), font_pat, FcMatchPattern);
    FcDefaultSubstitute(font_pat);

    FcResult result;
    FcPattern * matched_font_pat = FcFontMatch(_static_common->fontconfig_lib.get_config(), font_pat, &result);
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
        if(_lib_ref_cnt == 0)
            _static_common.reset();

        Logger_locator::get()(Logger::DBG, "Could not find font file matching: " + font_name);
        throw std::runtime_error("Could not find font file matching: " + font_name);
    }

    Logger_locator::get()(Logger::DBG, "Loading font file: " + font_file + " size: " + std::to_string(font_size));

    // open the found file
    FT_Error err = FT_New_Face(_static_common->ft_lib.get_lib(), font_file.c_str(), 0, &_face);

    if(err != FT_Err_Ok)
    {
        if(_lib_ref_cnt == 0)
            _static_common.reset();

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

    // select unicide charmap (should be default for most fonts)
    if(FT_Select_Charmap(_face, FT_ENCODING_UNICODE) != FT_Err_Ok)
    {
        FT_Done_Face(_face);

        if(_lib_ref_cnt == 0)
            _static_common.reset();

        Logger_locator::get()(Logger::ERROR, "No unicode charmap in font file: " + font_file);
        throw std::system_error(err, std::system_category(), "No unicode charmap in font file: " + font_file);
    }

    // select font size
    if(FT_Set_Char_Size(_face, font_size * 64, font_size * 64, h_dpi, v_dpi) != FT_Err_Ok)
    {
        std::cerr<<"error setting size"<<std::endl;

        FT_Done_Face(_face);

        if(_lib_ref_cnt == 0)
            _static_common.reset();

        Logger_locator::get()(Logger::ERROR, "Can't set font size: " + std::to_string(font_size) + " for font file: " + font_file);
        throw std::system_error(err, std::system_category(), "Can't set font size: " + std::to_string(font_size) + " for font file: " + font_file);
    }

    // get bounding box that will fit any glyph, plus 2 px padding
    // some glyphs overflow the reported box (antialiasing?) so at least one px is needed
    _cell_bbox.ul.x = FT_MulFix(_face->bbox.xMin, _face->size->metrics.x_scale) / 64 - 2;
    _cell_bbox.ul.y = FT_MulFix(_face->bbox.yMax, _face->size->metrics.y_scale) / 64 + 2;
    _cell_bbox.lr.x = FT_MulFix(_face->bbox.xMax, _face->size->metrics.x_scale) / 64 + 2;
    _cell_bbox.lr.y = FT_MulFix(_face->bbox.yMin, _face->size->metrics.y_scale) / 64 - 2;

    // get newline height
    _line_height = FT_MulFix(_face->height, _face->size->metrics.y_scale) / 64;

    _tex_width = _cell_bbox.width() * 16;
    _tex_height = _cell_bbox.height() * 16;

    _has_kerning_info = FT_HAS_KERNING(_face);

    // we're not going to throw now, so increment library ref count
    ++_lib_ref_cnt;

    // create and set up vertex array and buffer
    _vao.bind();
    _vbo.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (const GLvoid *)sizeof(glm::vec2));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // get shader uniform locations
    _static_common->prog.use();
    glUniform1i(_static_common->prog.get_uniform("font_page"), 14);
    glUseProgram(0);

    check_error("Font_sys::Font_sys");
}

// deallocate font
Font_sys::~Font_sys()
{
    FT_Done_Face(_face);

    // only deallocate shared libs if this is the last Font_sys obj
    if(--_lib_ref_cnt == 0)
        _static_common.reset();

    Logger_locator::get()(Logger::DBG, "Unloading font");
}

// render text (rebuilds for each frame - use Static_text if text doesn't change)
void Font_sys::render_text(const std::string & utf8_input, const glm::vec4 & color,
    const glm::vec2 & win_size, const glm::vec2 & pos, const int align_flags)
{
    // build text buffer objs
    Bbox<float> text_box;
    auto coord_data = build_text(utf8_input, *this, text_box);

    glm::vec2 start_offset = pos;

    // offset origin to align to text bounding box
    int horiz_align = align_flags & 0x3;
    switch(horiz_align)
    {
    case Font_sys::ORIGIN_HORIZ_BASELINE:
        break;
    case Font_sys::ORIGIN_HORIZ_LEFT:
        start_offset.x -= text_box.ul.x;
        break;
    case Font_sys::ORIGIN_HORIZ_RIGHT:
        start_offset.x -= text_box.lr.x;
        break;
    case Font_sys::ORIGIN_HORIZ_CENTER:
        start_offset.x -= text_box.ul.x + text_box.width() / 2.0f;
        break;
    }

    int vert_align = align_flags & 0xC;
    switch(vert_align)
    {
    case Font_sys::ORIGIN_VERT_BASELINE:
        break;
    case Font_sys::ORIGIN_VERT_TOP:
        start_offset.y -= text_box.ul.y;
        break;
    case Font_sys::ORIGIN_VERT_BOTTOM:
        start_offset.y -= text_box.lr.y;
        break;
    case Font_sys::ORIGIN_VERT_CENTER:
        start_offset.y -= text_box.lr.y + text_box.height() / 2.0f;
        break;
    }

    _vao.bind();
    _vbo.bind();

    // load text into buffer object
    // call glBufferData with NULL first - this is apparently faster for dynamic data loading
    glBufferData(_vbo.type(), sizeof(glm::vec2) * coord_data.first.size(), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(_vbo.type(), 0, sizeof(glm::vec2) * coord_data.first.size(), coord_data.first.data());

    // set up shader uniforms
    _static_common->prog.use();
    glUniform2fv(_static_common->prog.get_uniform("start_offset"), 1, &start_offset[0]);
    glUniform2fv(_static_common->prog.get_uniform("win_size"), 1, &win_size[0]);
    glUniform4fv(_static_common->prog.get_uniform("color"), 1, &color[0]);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE14);

    // draw text, per page
    for(const auto & cd: coord_data.second)
    {
        // bind the page's texture
        _page_map[cd.page_no].tex->bind();
        glDrawArrays(GL_TRIANGLES, cd.start, cd.num_elements);
    }

    glBindVertexArray(0);

    #ifdef DEBUG
    check_error("Font_sys::render_text");
    #endif
}

// create a font page texture
std::unordered_map<uint32_t, Font_sys::Page>::iterator Font_sys::load_page(const uint32_t page_no)
{
    std::ostringstream ostream;
    ostream<<"Loading font page "<<std::hex<<std::showbase<<page_no;
    Logger_locator::get()(Logger::TRACE, ostream.str());

    // this assumes the page has not been created yet
    auto page_i = _page_map.emplace(std::make_pair(page_no, Page())).first;
    Page & page = page_i->second;

    // greyscale pixel storage
    std::vector<char> tex_data(_tex_width * _tex_height, 0);

    FT_GlyphSlot slot = _face->glyph;

    // load each glyph in the page (256 per page)
    for(uint32_t code_pt = page_no << 8; code_pt < ((page_no + 1) << 8); code_pt++)
    {
        unsigned short tbl_row = (code_pt >> 4) & 0xF;
        unsigned short tbl_col = code_pt & 0xF;

        // have freetype render the glyph
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

        // set glyph properties
        c.origin.x = -_cell_bbox.ul.x + slot->bitmap_left;
        c.origin.y = _cell_bbox.ul.y - slot->bitmap_top;
        c.bbox.ul.x = slot->bitmap_left;
        c.bbox.ul.y = slot->bitmap_top;
        c.bbox.lr.x = (int)bmp->width + slot->bitmap_left;
        c.bbox.lr.y = slot->bitmap_top - (int)bmp->rows;
        c.advance.x = slot->advance.x;
        c.advance.y = slot->advance.y;
        c.glyph_i = glyph_i;

        // copy glyph from freetype to texture storage
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

    // copy data to a new opengl texture
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
