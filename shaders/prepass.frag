// prepass.frag
// fragment shader for scene prepass

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

#version 130

in vec2 tex_coord;
in vec3 normal_vec;
in vec3 tangent;

// material vars
struct Material
{
    float shininess;
    sampler2D normal_shininess_map;
};

uniform Material material;

out vec4 g_norm_shininess;

vec3 norm_map_normal(in vec3 normal, in vec3 tangent, in vec3 mapped_normal)
{
    normal = normalize(normal);
    tangent = normalize(tangent);

    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);

    mat3 tangent_bitangent_normal = mat3(tangent, bitangent, normal);

    mapped_normal = 2.0 * mapped_normal - vec3(1.0, 1.0, 1.0);

    vec3 new_normal = tangent_bitangent_normal * mapped_normal;
    return normalize(new_normal);
}

void main()
{
    vec4 normal_shininess = texture(material.normal_shininess_map, tex_coord);
    g_norm_shininess = vec4(norm_map_normal(normal_vec, tangent, normal_shininess.rgb),
        // shininess in alpha
        material.shininess * normal_shininess.a);
}
