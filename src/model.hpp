// model.hpp
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

#ifndef MODEL_HPP
#define MODEL_HPP

#include <functional>
#include <string>
#include <unordered_map>

#include <SFML/System.hpp>

#include "component.hpp"
#include "gl_wrappers.hpp"
#include "material.hpp"

class Model: public Component, public sf::NonCopyable
{
public:
    virtual ~Model();
    static std::shared_ptr<Model> create(const std::string & filename, const bool casts_shadow);
    virtual void draw(const std::function<void(const Material &)> & set_material) const;
protected:
    struct Mesh
    {
        GLsizei count = 0;
        std::size_t index = 0;
        GLint base_vert = 0;
        const Material * mat;
    };

    Model(const bool casts_shadow);
    Model(const std::string & filename, const bool casts_shadow);

    bool _casts_shadow = true;

    GL_vertex_array _vao;
    GL_buffer _vbo;
    GL_buffer _ebo;

    std::vector<Mesh> _meshes;
    std::vector<Material> _mats;

    std::string _key;
};

struct Model_cache
{
    std::unordered_map<std::string, std::shared_ptr<Model>> allocated_mdl;
};

class Model_cache_locator
{
public:
    Model_cache_locator() = delete;
    ~Model_cache_locator() = delete;

    static void init(std::shared_ptr<Model_cache> cache = std::make_shared<Model_cache>());
    static Model_cache & get();

private:
    static std::shared_ptr<Model_cache> _cache;
};

#endif // MODEL_HPP
