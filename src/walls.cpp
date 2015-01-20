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

#include "gl_helpers.hpp" // TODO: move w/ walls decl
#include "grid.hpp"
#include "walls.hpp"

void Walls::init()
{
    Grid grid(32, 32);
    grid.init();

    std::vector<glm::vec3> vert_pos;

    glm::vec3 cell_scale(1.0f, 1.0f, 1.0f);

    // draw border walls
    for(size_t row = 0; row < grid.grid.size(); ++row)
    {
        glm::vec3 origin(cell_scale.x * (float)grid.grid[row].size(), cell_scale.y * (float)row, cell_scale.z);

        vert_pos.push_back(origin);
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));

        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
        vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, cell_scale.z));
    }
    for(size_t col = 0; col < grid.grid[0].size(); ++col)
    {
        glm::vec3 origin(cell_scale.x * (float)col, cell_scale.y * (float)grid.grid.size(), cell_scale.z);
        vert_pos.push_back(origin);
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));

        vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
        vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, cell_scale.z));
    }

    // draw cell walls
    for(size_t row = 0; row < grid.grid.size(); ++row)
    {
        for(size_t col = 0; col < grid.grid[row].size(); ++col)
        {
            glm::vec3 origin(cell_scale.x * (float)col, cell_scale.y * (float)row, cell_scale.z);

            if(grid.grid[row][col].walls[UP])
            {
                vert_pos.push_back(origin);
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));

                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, 0.0f));
                vert_pos.push_back(origin + glm::vec3(cell_scale.x, 0.0f, cell_scale.z));
            }

            if(grid.grid[row][col].walls[LEFT])
            {
                vert_pos.push_back(origin);
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));

                vert_pos.push_back(origin + glm::vec3(0.0f, 0.0f, cell_scale.z));
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, 0.0f));
                vert_pos.push_back(origin + glm::vec3(0.0f, cell_scale.y, cell_scale.z));
            }

        }
    }

    _num_verts = vert_pos.size();

    // create OpenGL vertex objects
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vert_pos.size(), vert_pos.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    check_error("Walls::init");

    _prog.init({std::make_pair("shaders/walls.vert", GL_VERTEX_SHADER), std::make_pair("shaders/walls.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)});
    _prog.add_uniform("model_view_proj");
}

void Walls::draw(const Entity & cam, const glm::mat4 & proj)
{
    glUseProgram(_prog());
    glm::mat4 model_view_proj = proj * cam.view_mat();
    glUniformMatrix4fv(_prog.uniforms["model_view_proj"], 1, GL_FALSE, &model_view_proj[0][0]);
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, _num_verts);

    glBindVertexArray(0); // TODO: get prev val?
    glUseProgram(0); // TODO: get prev val?

    check_error("Walls::Draw");
}
