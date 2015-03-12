// model.cpp
// imported model component

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

#include "model.hpp"

#include <stdexcept>
#include <system_error>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <glm/glm.hpp>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "config.hpp"
#include "gl_helpers.hpp"
#include "logger.hpp"

std::unordered_map<std::string, std::weak_ptr<Model>> Model::_allocated_mdl;

Model::~Model()
{
    if(_key.size() > 0)
        _allocated_mdl.erase(_key);
}

std::shared_ptr<Model> Model::create(const std::string & filename)
{
    if(Model::_allocated_mdl.count(filename) > 0)
    {
        Logger_locator::get()(Logger::DBG, "Reusing model: " + filename);
        return _allocated_mdl[filename].lock();
    }
    else
    {
        std::shared_ptr<Model> ret(new Model(filename));
        ret->_key = "MDL:" + filename;
        _allocated_mdl[filename] = ret;
        return ret;
    }
}

void Model::draw(const std::function<void(const Material &)> & set_material) const
{
    _vao.bind();

    for(const auto & mesh: _meshes)
    {
        set_material(*mesh.mat);
        glDrawElementsBaseVertex(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (GLvoid *)mesh.index, mesh.base_vert);
    }

    glBindVertexArray(0); // TODO: get prev val?

    check_error("Model::Draw");
}

Model::Model():
    _vbo(GL_ARRAY_BUFFER),
    _ebo(GL_ELEMENT_ARRAY_BUFFER)
{
}

Model::Model(const std::string & filename):
    _vbo(GL_ARRAY_BUFFER),
    _ebo(GL_ELEMENT_ARRAY_BUFFER)
{
    Logger_locator::get()(Logger::DBG, "Loading model: " + filename);
    // TODO: check format (COLLADA)
    Assimp::Importer imp;

    // TODO: remove unneeded flags
    const aiScene * ai_scene = imp.ReadFile(filename,
        aiProcess_CalcTangentSpace | aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
        aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);

    if(!ai_scene)
    {
        Logger_locator::get()(Logger::ERROR, "Could not load model (" + filename + "): " + imp.GetErrorString());
        throw std::ios_base::failure("Could not load model (" + filename + "): " + imp.GetErrorString());
    }

    if(!ai_scene->HasMeshes())
    {
        Logger_locator::get()(Logger::ERROR, "no meshes");
        throw std::runtime_error("no meshes");
    }

    // get material
    for(std::size_t mat_i = 0; mat_i < ai_scene->mNumMaterials; ++mat_i)
    {
        const aiMaterial * ai_mat = ai_scene->mMaterials[mat_i];

        _mats.emplace_back();
        Material & mat = _mats.back();

        aiColor3D mat_color;
        float mat_val;

        if(ai_mat->Get(AI_MATKEY_COLOR_AMBIENT, mat_color) == AI_SUCCESS)
        {
            mat.ambient_color = glm::vec3(mat_color.r, mat_color.g, mat_color.b);
        }

        if(ai_mat->Get(AI_MATKEY_COLOR_DIFFUSE, mat_color) == AI_SUCCESS)
        {
            mat.diffuse_color = glm::vec3(mat_color.r, mat_color.g, mat_color.b);
        }

        if(ai_mat->Get(AI_MATKEY_COLOR_SPECULAR, mat_color) == AI_SUCCESS)
        {
            mat.specular_color = glm::vec3(mat_color.r, mat_color.g, mat_color.b);
        }

        if(ai_mat->Get(AI_MATKEY_SHININESS, mat_val) == AI_SUCCESS)
        {
            mat.shininess = mat_val;
        }

        if(ai_mat->Get(AI_MATKEY_COLOR_EMISSIVE, mat_color) == AI_SUCCESS)
        {
            mat.emissive_color = glm::vec3(mat_color.r, mat_color.g, mat_color.b);
        }

        // Assimp doesn't have a material reflectivity parameter

        aiString tex_path;
        if(ai_mat->GetTexture(aiTextureType_AMBIENT, 0, &tex_path) == AI_SUCCESS)
        {
            mat.ambient_map = Texture_2D::create(check_in_pwd(std::string("mdl/") + tex_path.C_Str()));
        }

        if(ai_mat->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == AI_SUCCESS)
        {
            mat.diffuse_map = Texture_2D::create(check_in_pwd(std::string("mdl/") + tex_path.C_Str()));
        }

        if(ai_mat->GetTexture(aiTextureType_SPECULAR, 0, &tex_path) == AI_SUCCESS)
        {
            mat.specular_map = Texture_2D::create(check_in_pwd(std::string("mdl/") + tex_path.C_Str()));
        }

        if(ai_mat->GetTexture(aiTextureType_SHININESS, 0, &tex_path) == AI_SUCCESS)
        {
            mat.shininess_map = Texture_2D::create(check_in_pwd(std::string("mdl/") + tex_path.C_Str()));
        }

        if(ai_mat->GetTexture(aiTextureType_EMISSIVE, 0, &tex_path) == AI_SUCCESS)
        {
            mat.emissive_map = Texture_2D::create(check_in_pwd(std::string("mdl/") + tex_path.C_Str()));
            mat.emissive_color = glm::vec3(1.0f); // Blender doesn't set this correctly
        }

        // if(ai_mat->GetTexture(aiTextureType_REFLECTION, 0, &tex_path) == AI_SUCCESS)
        // {
        //     mat.reflectivity_map = Texture_2D::create(check_in_pwd(std::string("mdl/") + tex_path.C_Str()));
        // }

        if(ai_mat->GetTexture(aiTextureType_NORMALS, 0, &tex_path) == AI_SUCCESS)
        {
            mat.normal_map = Texture_2D::create(check_in_pwd(std::string("mdl/") + tex_path.C_Str()));
        }
    }

    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec2> vert_tex_coords;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::vec3> vert_tangents;
    std::vector<GLuint> index;

    glm::mat3 rot_mat(glm::rotate(glm::mat4(1.0f), -0.5f * (float)M_PI, glm::vec3(1.0f, 0.0f, 0.0f))); // converts from Z-up to Y-up

    for(std::size_t mesh_i = 0; mesh_i < ai_scene->mNumMeshes; ++mesh_i)
    {
        const aiMesh * ai_mesh = ai_scene->mMeshes[mesh_i];

        if(!ai_mesh->HasTextureCoords(0))
        {
            Logger_locator::get()(Logger::ERROR, filename + " - mesh[" + std::to_string(mesh_i) + "] has no tex coords");
            throw std::runtime_error(filename + " - mesh[" + std::to_string(mesh_i) + "] has no tex coords");
        }
        if(!ai_mesh->HasNormals())
        {
            Logger_locator::get()(Logger::ERROR, filename + " - mesh[" + std::to_string(mesh_i) + "] has no normals");
            throw std::runtime_error(filename + " - mesh[" + std::to_string(mesh_i) + "] has no normals");
        }

        // Tangets are generated, so we don't need to check them

        _meshes.emplace_back();
        Mesh & mesh = _meshes.back();

        mesh.index = sizeof(GLint) * index.size();
        mesh.base_vert = vert_pos.size();

        if(ai_mesh->mMaterialIndex < _mats.size())
            mesh.mat = &_mats[ai_mesh->mMaterialIndex];
        else
        {
            Logger_locator::get()(Logger::ERROR, filename + " - mesh[" + std::to_string(mesh_i) + "] refers to non-existant material: " + std::to_string(ai_mesh->mMaterialIndex));
            throw std::runtime_error(filename + " - mesh[" + std::to_string(mesh_i) + "] refers to non-existant material: " + std::to_string(ai_mesh->mMaterialIndex));
            return;
        }

        for(std::size_t vert_i = 0; vert_i < ai_mesh->mNumVertices; ++vert_i)
        {
            const aiVector3D & ai_vert = ai_mesh->mVertices[vert_i];
            vert_pos.push_back(rot_mat * glm::vec3(ai_vert.x, ai_vert.y, ai_vert.z));

            const aiVector3D & ai_tex = ai_mesh->mTextureCoords[0][vert_i];
            vert_tex_coords.push_back(glm::vec2(ai_tex.x, ai_tex.y));

            const aiVector3D & ai_norm = ai_mesh->mNormals[vert_i];
            vert_normals.push_back(rot_mat * glm::vec3(ai_norm.x, ai_norm.y, ai_norm.z));

            const aiVector3D & ai_tangent = ai_mesh->mTangents[vert_i];
            vert_tangents.push_back(rot_mat * glm::vec3(ai_tangent.x, ai_tangent.y, ai_tangent.z));
        }

        // get indexes
        for(std::size_t face_i = 0; face_i < ai_mesh->mNumFaces; ++face_i)
        {
            const aiFace & ai_face = ai_mesh->mFaces[face_i];
            if(ai_face.mNumIndices != 3)
            {
                Logger_locator::get()(Logger::ERROR, filename + " - mesh[" + std::to_string(mesh_i) + "] has non-triangular face: " + std::to_string(face_i));
                throw std::runtime_error(filename + " - mesh[" + std::to_string(mesh_i) + "] has non-triangular face: " + std::to_string(face_i));
            }

            for(std::size_t i = 0; i < ai_face.mNumIndices; ++i)
                index.push_back(ai_face.mIndices[i]);

            mesh.count += ai_face.mNumIndices;
        }
    }

    // create OpenGL vertex objects
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

    _ebo.bind();
    glBufferData(_ebo.type(), sizeof(GLuint) * index.size(), index.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    check_error("Model::Model");
}
