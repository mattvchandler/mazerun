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

// TODO: default maps (white for diffuse, blue for normal, black for emmissive)
struct Material
{
    // TODO: ambient term?
    // TODO: diffuse color term?
    // TODO: specular map?
    glm::vec3 specular_color;
    std::shared_ptr<Texture_2D> diffuse_map;
    std::shared_ptr<Texture_2D> normal_map;
    std::shared_ptr<Texture_2D> emissive_map;
    float shininess; // TODO: add as alpha to normal map (https://developer.valvesoftware.com/wiki/Phong)
        // TODO: shininess is exponent
    // TODO: exponent?
    // TODO: ctor w/ tex vars
    // TODO: environt mapping (reflectivity map?)
};

#endif // MATERIAL_HPP
