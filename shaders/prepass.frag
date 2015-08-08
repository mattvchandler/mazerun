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
    sampler2D shininess_map; // greyscale
    sampler2D normal_map;
};

uniform Material material;

out vec4 g_shininess;
out vec4 g_norm;

vec3 norm_map_normal(in vec3 normal, in vec3 tangent, in sampler2D normal_map, in vec2 tex_coord)
{
    normal = normalize(normal);
    tangent = normalize(tangent);

    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);

    mat3 tangent_bitangent_normal = mat3(tangent, bitangent, normal);

    vec3 map_normal = texture(normal_map, tex_coord).xyz;
    map_normal = 2.0 * map_normal - vec3(1.0, 1.0, 1.0);

    vec3 new_normal = tangent_bitangent_normal * map_normal;
    return normalize(new_normal);
}

void main()
{
    g_shininess = vec4(material.shininess * texture(material.shininess_map, tex_coord).r, 0.0, 0.0, 1.0);
    g_norm = vec4(norm_map_normal(normal_vec, tangent, material.normal_map, tex_coord), 1.0);
}
