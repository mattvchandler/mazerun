// walls.cpp
// maze walls

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

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "config.hpp"
#include "gl_helpers.hpp"
#include "grid.hpp"
#include "walls.hpp"

Walls::Walls(): _vbo(GL_ARRAY_BUFFER)
{
}

void Walls::init(const unsigned int width, const unsigned int height)
{
    Grid grid(width, height);
    grid.init();

    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec2> vert_tex_coords;
    std::vector<glm::vec3> vert_normals;

    glm::vec3 cell_scale(1.0f, 1.0f, 1.0f);
    glm::vec3 base(-0.5f * (float)grid.grid[0].size(), -0.5f * (float)grid.grid.size(), 0.0f);

    glm::vec3 left_normal(-1.0f, 0.0f, 0.0f);
    glm::vec3 up_normal(0.0f, -1.0f, 0.0f);

    // draw border walls
    for(size_t row = 0; row < grid.grid.size(); ++row)
    {
        glm::vec3 origin(cell_scale.x * (float)grid.grid[row].size(), cell_scale.y * (float)row, 0.0f);
        origin += base;

        vert_pos.push_back(origin);
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));

        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, cell_scale.z));

        for(int i = 0; i < 6; ++i)
            vert_normals.push_back(left_normal);
    }
    for(size_t col = 0; col < grid.grid[0].size(); ++col)
    {
        glm::vec3 origin(cell_scale.x * (float)col, cell_scale.y * (float)grid.grid.size(), 0.0f);
        origin += base;

        vert_pos.push_back(origin);
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));

        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, cell_scale.z));

        for(int i = 0; i < 6; ++i)
            vert_normals.push_back(up_normal);
    }

    // draw cell walls
    for(size_t row = 0; row < grid.grid.size(); ++row)
    {
        for(size_t col = 0; col < grid.grid[row].size(); ++col)
        {
            glm::vec3 origin(cell_scale.x * (float)col, cell_scale.y * (float)row, 0.0f);
            origin += base;

            if(grid.grid[row][col].walls[UP])
            {
                vert_pos.push_back(origin);
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));

                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, cell_scale.z));

                for(int i = 0; i < 6; ++i)
                    vert_normals.push_back(up_normal);
            }

            if(grid.grid[row][col].walls[LEFT])
            {
                vert_pos.push_back(origin);
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));

                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, cell_scale.z));

                for(int i = 0; i < 6; ++i)
                    vert_normals.push_back(left_normal);
            }
        }
    }

    // add tex coords
    for(size_t i = 0; i < vert_pos.size(); i += 6)
    {
        vert_tex_coords.push_back(glm::vec2(0.0f, 0.0f));
        vert_tex_coords.push_back(glm::vec2(1.0f, 0.0f));
        vert_tex_coords.push_back(glm::vec2(0.0f, 1.0f));

        vert_tex_coords.push_back(glm::vec2(0.0f, 1.0f));
        vert_tex_coords.push_back(glm::vec2(1.0f, 0.0f));
        vert_tex_coords.push_back(glm::vec2(1.0f, 1.0f));
    }

    _num_verts = vert_pos.size();

    // create OpenGL vertex objects
    _vao.gen(); _vao.bind();
    _vbo.gen(); _vbo.bind();

    glBufferData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size() + sizeof(glm::vec3) * vert_normals.size(), NULL, GL_STATIC_DRAW);
    glBufferSubData(_vbo.type(), 0, sizeof(glm::vec3) * vert_pos.size(), vert_pos.data());
    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size(),
        sizeof(glm::vec2) * vert_tex_coords.size(), vert_tex_coords.data());
    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size(), sizeof(glm::vec3) * vert_normals.size(), vert_normals.data());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size()));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size()));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    _mat.tex.init(check_in_pwd("img/GroundCover.jpg"));
    _mat.shininess = 50.0f;
    _mat.specular_color = glm::vec3(1.0f, 1.0f, 1.0f);

    check_error("Walls::init");
}

void Walls::draw() const
{
    _vao.bind();

    glDrawArrays(GL_TRIANGLES, 0, _num_verts);
    glBindVertexArray(0); // TODO: get prev val?

    check_error("Walls::Draw");
}

const Material & Walls::get_material() const
{
    return _mat;
}

Floor::Floor(): _vbo(GL_ARRAY_BUFFER)
{
}

void Floor::init(const unsigned int width, const unsigned int height)
{
    glm::vec2 ll(-0.5f * (float)width, -0.5f * (float)height);
    glm::vec2 ur(0.5f * (float)width, 0.5f * (float)height);

    std::vector<glm::vec3> vert_pos =
    {
        glm::vec3(ll, 0.0f),
        glm::vec3(ur.x, ll.y, 0.0f),
        glm::vec3(ll.x, ur.y, 0.0f),
        glm::vec3(ur, 0.0f)
    };

    std::vector<glm::vec2> vert_tex_coords =
    {
        glm::vec2(0.0f, 0.0f),
        glm::vec2((float)width, 0.0f),
        glm::vec2(0.0f, (float)height),
        glm::vec2((float)width, (float)height)
    };

    std::vector<glm::vec3> vert_normals =
    {
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };

    _num_verts = vert_pos.size();

    // create OpenGL vertex objects
    _vao.gen(); _vao.bind();
    _vbo.gen(); _vbo.bind();

    glBufferData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size() + sizeof(glm::vec3) * vert_normals.size(), NULL, GL_STATIC_DRAW);
    glBufferSubData(_vbo.type(), 0, sizeof(glm::vec3) * vert_pos.size(), vert_pos.data());
    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size(),
        sizeof(glm::vec2) * vert_tex_coords.size(), vert_tex_coords.data());
    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size(), sizeof(glm::vec3) * vert_normals.size(), vert_normals.data());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size()));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size()));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    _mat.tex.init(check_in_pwd("img/AncientFlooring.jpg"));
    _mat.shininess = 50.0f;
    _mat.specular_color = glm::vec3(1.0f, 1.0f, 1.0f);

    check_error("Floor::init");
}

void Floor::draw() const
{
    _vao.bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, _num_verts);

    glBindVertexArray(0); // TODO: get prev val?

    check_error("Floor::Draw");
}

const Material & Floor::get_material() const
{
    return _mat;
}
