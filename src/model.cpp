// model.cpp
// imported model class

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

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "config.hpp"
#include "gl_helpers.hpp"
#include "model.hpp"

Model::Model(const std::string & filename):
    _filename(filename), _ebo(GL_ELEMENT_ARRAY_BUFFER)
{
}

void Model::init()
{
    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec2> vert_tex_coords;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::vec3> vert_tangents;
    std::vector<GLuint> index;

    Assimp::Importer imp;

    const aiScene * scene = imp.ReadFile(_filename,
        aiProcess_CalcTangentSpace | aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
        aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);

    if(!scene)
    {
        // TODO: throw
        return;
    }

    for(size_t mesh_i = 0; mesh_i < scene->mNumMeshes; ++mesh_i)
    {
        aiMesh * mesh = scene->mMeshes[mesh_i];
        for(size_t vert_i = 0; vert_i < mesh->mNumVertices; ++vert_i)
        {
            // TODO: fallbacks
            aiVector3D & vert = mesh->mVertices[vert_i];
            vert_pos.push_back(glm::vec3(vert.x, vert.y, vert.z));

            if(mesh->HasTextureCoords(0))
            {
                aiVector3D & tex = mesh->mTextureCoords[0][vert_i];
                vert_tex_coords.push_back(glm::vec2(tex.x, tex.y));
            }
            else
            {
                vert_tex_coords.push_back(glm::vec2(0.0f));
            }

            if(mesh->HasNormals())
            {
                aiVector3D & norm = mesh->mNormals[vert_i];
                vert_normals.push_back(glm::vec3(norm.x, norm.y, norm.z));
            }
            else
            {
                vert_normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
            }

            if(mesh->HasTangentsAndBitangents())
            {
                aiVector3D & tangent = mesh->mTangents[vert_i];
                vert_tangents.push_back(glm::vec3(tangent.x, tangent.y, tangent.z));
            }
            {
                vert_tangents.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }

        for(size_t face_i = 0; face_i < mesh->mNumFaces; ++face_i)
        {
            aiFace & face = mesh->mFaces[face_i];
            if(face.mNumIndices != 3)
            {
                // TODO: throw
                return;
            }

            for(size_t i = 0; i < face.mNumIndices; ++i)
                index.push_back(face.mIndices[i]);
        }
    }

    _num_verts = index.size();

    // create OpenGL vertex objects
    _vao.gen(); _vao.bind();
    _vbo.gen(); _vbo.bind();

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

    _ebo.gen(); _ebo.bind();
    glBufferData(_ebo.type(), sizeof(GLuint) * index.size(), index.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    _mat.emission_color = glm::vec3(0.0f, 0.0f, 0.0f);
    _mat.specular_color = glm::vec3(1.0f, 1.0f, 1.0f);
    _mat.diffuse_map.init(check_in_pwd("img/AncientFlooring.jpg"));
    _mat.normal_map.init(check_in_pwd("img/normals/AncientFlooring_N.jpg"));
    _mat.shininess = 1000.0f;

    check_error("Model::init");
}

void Model::draw() const
{
    _vao.bind();

    glDrawElements(GL_TRIANGLES, _num_verts, GL_UNSIGNED_INT, (GLvoid *)0);

    glBindVertexArray(0); // TODO: get prev val?

    check_error("Model::Draw");
}
