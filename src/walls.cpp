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

#include "walls.hpp"

#include <glm/glm.hpp>

#include "config.hpp"
#include "gl_helpers.hpp"

Walls::Walls(const unsigned int width, const unsigned int height):
    _grid(width, height, Grid::MAZEGEN_DFS, 25, 100),
    _vbo(GL_ARRAY_BUFFER)
{

    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec2> vert_tex_coords;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::vec3> vert_tangents;

    glm::vec3 cell_scale(1.0f, 1.0f, 1.0f);
    glm::vec3 base(-0.5f * (float)_grid.grid[0].size(), 0.0f, -0.5f * (float)_grid.grid.size());

    glm::vec3 left_normal(1.0f, 0.0f, 0.0f);
    glm::vec3 left_tangent(0.0f, 0.0f, -1.0f);
    glm::vec3 up_normal(0.0f, 0.0f, 1.0f);
    glm::vec3 up_tangent(1.0f, 0.0f, 0.0f);

    // draw border walls
    for(std::size_t col = 0; col < _grid.grid[0].size(); ++col)
    {
        glm::vec3 origin(cell_scale.x * (float)col, 0.0f, cell_scale.z * (float)_grid.grid.size());
        origin += base;

        vert_pos.push_back(origin);
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));

        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, cell_scale.y, 0.0f));

        for(int i = 0; i < 6; ++i)
        {
            vert_normals.push_back(up_normal);
            vert_tangents.push_back(up_tangent);
        }
    }
    for(std::size_t row = 0; row < _grid.grid.size(); ++row)
    {
        glm::vec3 origin(cell_scale.x * (float)_grid.grid[row].size(), 0.0f, cell_scale.z * (float)row);
        origin += base;

        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
        vert_pos.push_back(origin);
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, cell_scale.z));

        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, cell_scale.z));
        vert_pos.push_back(origin);
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));

        for(int i = 0; i < 6; ++i)
        {
            vert_normals.push_back(left_normal);
            vert_tangents.push_back(left_tangent);
        }
    }

    // draw cell walls
    for(std::size_t row = 0; row < _grid.grid.size(); ++row)
    {
        for(std::size_t col = 0; col < _grid.grid[row].size(); ++col)
        {
            glm::vec3 origin(cell_scale.x * (float)col, 0.0f, cell_scale.y * (float)row);
            origin += base;

            if(_grid.grid[row][col].walls[UP])
            {
                vert_pos.push_back(origin);
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));

                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, cell_scale.y, 0.0f));

                for(int i = 0; i < 6; ++i)
                {
                    vert_normals.push_back(up_normal);
                    vert_tangents.push_back(up_tangent);
                }
            }

            if(_grid.grid[row][col].walls[LEFT])
            {
                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
                vert_pos.push_back(origin);
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, cell_scale.z));

                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, cell_scale.z));
                vert_pos.push_back(origin);
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));

                for(int i = 0; i < 6; ++i)
                {
                    vert_normals.push_back(left_normal);
                    vert_tangents.push_back(left_tangent);
                }
            }
        }
    }

    // add tex coords
    for(std::size_t i = 0; i < vert_pos.size(); i += 6)
    {
        vert_tex_coords.push_back(glm::vec2(0.0f, 0.0f));
        vert_tex_coords.push_back(glm::vec2(1.0f, 0.0f));
        vert_tex_coords.push_back(glm::vec2(0.0f, 1.0f));

        vert_tex_coords.push_back(glm::vec2(0.0f, 1.0f));
        vert_tex_coords.push_back(glm::vec2(1.0f, 0.0f));
        vert_tex_coords.push_back(glm::vec2(1.0f, 1.0f));
    }

    _num_verts = vert_pos.size();

    _vao.bind();
    _vbo.bind();

    glBufferData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size() +
        sizeof(glm::vec3) * vert_normals.size() +
        sizeof(glm::vec3) * vert_tangents.size(), NULL, GL_STATIC_DRAW);

    glBufferSubData(_vbo.type(), 0, sizeof(glm::vec3) * vert_pos.size(), vert_pos.data());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size(),
        sizeof(glm::vec2) * vert_tex_coords.size(), vert_tex_coords.data());
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size()));
    glEnableVertexAttribArray(1);

    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size(),
        sizeof(glm::vec3) * vert_normals.size(), vert_normals.data());
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size()));
    glEnableVertexAttribArray(2);

    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size() +
        sizeof(glm::vec3) * vert_normals.size(),
        sizeof(glm::vec3) * vert_tangents.size(), vert_tangents.data());
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size() +
        sizeof(glm::vec3) * vert_normals.size()));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    _mat.specular_color = glm::vec3(0.1f, 0.1f, 0.1f);
    _mat.diffuse_map = Texture_2D::create(check_in_pwd("img/GroundCover.jpg"));
    _mat.normal_map = Texture_2D::create(check_in_pwd("img/normals/GroundCover_N.jpg"));
    _mat.shininess = 500.0f;

    check_error("Walls::Walls");
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

Floor::Floor(const unsigned int width, const unsigned int height):
    _vbo(GL_ARRAY_BUFFER)
{
    glm::vec2 ll(-0.5f * (float)width, 0.5f * (float)height);
    glm::vec2 ur(0.5f * (float)width, -0.5f * (float)height);

    std::vector<glm::vec3> vert_pos =
    {
        glm::vec3(ll.x, 0.0f, ll.y),
        glm::vec3(ur.x, 0.0f, ll.y),
        glm::vec3(ll.x, 0.0f, ur.y),
        glm::vec3(ur.x, 0.0f, ur.y)
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
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };

    std::vector<glm::vec3> vert_tangents =
    {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    };

    _num_verts = vert_pos.size();

    _vao.bind();
    _vbo.bind();

    glBufferData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size() +
        sizeof(glm::vec3) * vert_normals.size() +
        sizeof(glm::vec3) * vert_tangents.size(), NULL, GL_STATIC_DRAW);

    glBufferSubData(_vbo.type(), 0, sizeof(glm::vec3) * vert_pos.size(), vert_pos.data());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size(),
        sizeof(glm::vec2) * vert_tex_coords.size(), vert_tex_coords.data());
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size()));
    glEnableVertexAttribArray(1);

    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size(),
        sizeof(glm::vec3) * vert_normals.size(), vert_normals.data());
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size()));
    glEnableVertexAttribArray(2);

    glBufferSubData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size() +
        sizeof(glm::vec3) * vert_normals.size(),
        sizeof(glm::vec3) * vert_tangents.size(), vert_tangents.data());
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * vert_pos.size() +
        sizeof(glm::vec2) * vert_tex_coords.size() +
        sizeof(glm::vec3) * vert_normals.size()));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    _mat.specular_color = glm::vec3(0.1f, 0.1f, 0.1f);
    _mat.diffuse_map = Texture_2D::create(check_in_pwd("mdl/AncientFlooring.jpg"));
    _mat.normal_map = Texture_2D::create(check_in_pwd("mdl/AncientFlooring_N.jpg"));
    _mat.shininess = 500.0f;

    check_error("Floor::Floor");
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
