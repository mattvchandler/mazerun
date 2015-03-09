// skybox.cpp
// Skybox info

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

#include "skybox.hpp"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "config.hpp"
#include "logger.hpp"

// TODO: replace bottom half of skybox with ground (if doing environment mapping)

Skybox::Skybox():
    _vbo(GL_ARRAY_BUFFER),
    _ebo(GL_ELEMENT_ARRAY_BUFFER),
    _tex(Texture_cubemap::create(check_in_pwd("img/bluecloud_lf.jpg"), check_in_pwd("img/bluecloud_rt.jpg"),
        check_in_pwd("img/bluecloud_bk.jpg"), check_in_pwd("img/bluecloud_ft.jpg"),
        check_in_pwd("img/bluecloud_dn.jpg"), check_in_pwd("img/bluecloud_up.jpg"))),
    _prog({std::make_pair("shaders/skybox.vert", GL_VERTEX_SHADER), std::make_pair("shaders/skybox.frag", GL_FRAGMENT_SHADER)},
        {std::make_pair("vert_pos", 0)})
{
    // TODO: a sphere might look better
    std::vector<glm::vec3> vert_pos =
    {
        glm::vec3(-1.0f, -1.0f, -1.0f), // 0
        glm::vec3(1.0f, -1.0f, -1.0f), // 1
        glm::vec3(-1.0f, -1.0f, 1.0f), // 2
        glm::vec3(1.0f, -1.0f, 1.0f), // 3
        glm::vec3(-1.0f, 1.0f, -1.0f), // 4
        glm::vec3(1.0f, 1.0f, -1.0f), // 5
        glm::vec3(-1.0f, 1.0f, 1.0f), // 6
        glm::vec3(1.0f, 1.0f, 1.0f) // 7
    };

    std::vector<GLuint> index =
    {
        // front
        0, 1, 5,
        0, 5, 4,
        // right
        1, 3, 7,
        1, 7, 5,
        // back
        3, 2, 6,
        3, 6, 7,
        // left
        2, 0, 4,
        2, 4, 6,
        // top
        4, 5, 7,
        4, 7, 6,
        // bottom
        2, 3, 1,
        2, 1, 0
    };

    // create OpenGL vertex objects
    _vao.bind();
    _vbo.bind();

    glBufferData(_vbo.type(), sizeof(glm::vec3) * vert_pos.size(), vert_pos.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    _ebo.bind();
    glBufferData(_ebo.type(), sizeof(GLuint) * index.size(), index.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    _num_indexes = index.size();

    _prog.use();
    _prog.add_uniform("model_view_proj");

    glUseProgram(0); // TODO get prev val

    check_error("Skybox::Skybox");
}

void Skybox::draw(const Entity & cam, const glm::mat4 & proj)
{
    GLint depth_func_old;
    glGetIntegerv(GL_DEPTH_FUNC, &depth_func_old);

    glDepthFunc(GL_LEQUAL);

    _prog.use();
    _tex->bind();

    glm::mat4 model_view_proj = proj * glm::translate(cam.view_mat(), cam.pos());

    glUniformMatrix4fv(_prog.get_uniform("model_view_proj"), 1, GL_FALSE, &model_view_proj[0][0]);

    glActiveTexture(GL_TEXTURE0);
    _vao.bind();
    glDrawElements(GL_TRIANGLES, _num_indexes, GL_UNSIGNED_INT, (GLvoid *)0);

    glBindVertexArray(0); // get prev val?
    glUseProgram(0); // get prev val?

    glDepthFunc(depth_func_old);

    check_error("Skybox::draw");
}
