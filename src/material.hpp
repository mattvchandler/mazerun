// material.hpp
// material structures

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

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <memory>

#include <glm/glm.hpp>

#include "texture.hpp"

struct Material
{
    glm::vec3 ambient_color{1.0f, 1.0f, 1.0f};
    glm::vec3 diffuse_color{1.0f, 1.0f, 1.0f};
    glm::vec3 specular_color{1.0f, 1.0f, 1.0f};
    float shininess = 0.0f;
    glm::vec3 emissive_color{0.0f, 0.0f, 0.0f};
    // float reflectivity;

    std::shared_ptr<Texture_2D> ambient_map = Texture_2D::white_fallback();
    std::shared_ptr<Texture_2D> diffuse_map = Texture_2D::diffuse_map_fallback();
    std::shared_ptr<Texture_2D> specular_map = Texture_2D::white_fallback();
    std::shared_ptr<Texture_2D> shininess_map = Texture_2D::white_fallback(); // greyscale
    std::shared_ptr<Texture_2D> emissive_map = Texture_2D::white_fallback();
    // std::shared_ptr<Texture_2D> reflectivity_map = Texture_2D::white_fallback(); // greyscale
    std::shared_ptr<Texture_2D> normal_map = Texture_2D::normal_map_fallback();
};

#endif // MATERIAL_HPP
