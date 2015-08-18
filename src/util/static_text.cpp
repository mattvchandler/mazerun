// static_text.cpp
// static text object

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

#include "util/static_text.hpp"

#include "opengl/gl_helpers.hpp"
#include "util/logger.hpp"

// create and build text buffer object
Static_text::Static_text(Font_sys & font, const std::string & utf8_input,
    const glm::vec4 & color):
    _vbo(GL_ARRAY_BUFFER),
    _color(color)
{
    Logger_locator::get()(Logger::DBG, "Creating static text");

    // build the text
    auto coord_data = build_text(utf8_input, font, _text_box);

    _coord_data = coord_data.second;

    // set up buffer obj properties, load vertex data
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

// recreate text object with new string
void Static_text::set_text(Font_sys & font, const std::string & utf8_input)
{
    // build the text
    auto coord_data = build_text(utf8_input, font, _text_box);

    _coord_data = coord_data.second;

    _vao.bind();
    _vbo.bind();
    // reload vertex data
    glBufferData(_vbo.type(), sizeof(glm::vec2) * coord_data.first.size(),
        coord_data.first.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    check_error("Static_text::set_text");
}

// set font color
void Static_text::set_color(const glm::vec4 & color)
{
    _color = color;
}

// render the text
void Static_text::render_text(Font_sys & font, const glm::vec2 & win_size,
    const glm::vec2 & pos, const int align_flags)
{
    glm::vec2 start_offset = pos;

    int horiz_align = align_flags & 0x3;

    // offset origin to align to text bounding box
    switch(horiz_align)
    {
    case Font_sys::ORIGIN_HORIZ_BASELINE:
        break;
    case Font_sys::ORIGIN_HORIZ_LEFT:
        start_offset.x -= _text_box.ul.x;
        break;
    case Font_sys::ORIGIN_HORIZ_RIGHT:
        start_offset.x -= _text_box.lr.x;
        break;
    case Font_sys::ORIGIN_HORIZ_CENTER:
        start_offset.x -= _text_box.ul.x + _text_box.width() / 2.0f;
        break;
    }

    int vert_align = align_flags & 0xC;
    switch(vert_align)
    {
    case Font_sys::ORIGIN_VERT_BASELINE:
        break;
    case Font_sys::ORIGIN_VERT_TOP:
        start_offset.y -= _text_box.ul.y;
        break;
    case Font_sys::ORIGIN_VERT_BOTTOM:
        start_offset.y -= _text_box.lr.y;
        break;
    case Font_sys::ORIGIN_VERT_CENTER:
        start_offset.y -= _text_box.lr.y + _text_box.height() / 2.0f;
        break;
    }

    // set up shader uniforms
    font._static_common->prog.use();
    glUniform2fv(font._static_common->prog.get_uniform("start_offset"), 1, &start_offset[0]);
    glUniform2fv(font._static_common->prog.get_uniform("win_size"), 1, &win_size[0]);
    glUniform4fv(font._static_common->prog.get_uniform("color"), 1, &_color[0]);

    _vao.bind();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE13);

    // draw text, per page
    for(const auto & cd: _coord_data)
    {
        // bind the page's texture
        font._page_map[cd.page_no].tex->bind();
        glDrawArrays(GL_TRIANGLES, cd.start, cd.num_elements);
    }

    glBindVertexArray(0);

    #ifdef DEBUG
    check_error("Static_text::render_text");
    #endif
}
