// lighting.frag
// phong lighting fragment shader calculations
// to be linked with another frag shader file

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

// material vars
struct Material
{
    vec3 specular_color;
    vec3 emission_color;
    float shininess;
    sampler2D diffuse_map;
    sampler2D normal_map;
};

// lighting vars
struct Base_light
{
    vec3 color;
    float strength;
};

struct Point_light
{
    Base_light base;
    vec3 pos_eye;
    float const_atten;
    float linear_atten;
    float quad_atten;
};

struct Dir_light
{
    Base_light base;
    vec3 dir;
    vec3 half_vec;
};

vec3 norm_map_normal(in vec2 tex_coord, in vec3 normal, in vec3 tangent, in sampler2D normal_map)
{
    normal = normalize(normal);
    tangent = normalize(tangent);

    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);

    vec3 norm_normal = texture(normal_map, tex_coord).xyz;
    norm_normal = 2.0 * norm_normal - vec3(1.0, 1.0, 1.0);

    mat3 tangent_bitangent_normal = mat3(tangent, bitangent, normal);

    vec3 new_norm = tangent_bitangent_normal * norm_normal;
    return normalize(new_norm);
}

void calc_common_lighting(in vec3 normal_vec, in vec3 dir, in vec3 half_vec,
    in Material material, in Base_light base,
    out float diffuse_mul, out float specular_mul)
{
    // calculate ammt of diffuse and specular shading
    // TODO: remove
    if(gl_FrontFacing)
    {
        diffuse_mul = max(0.0, dot(normal_vec, normalize(dir)));
        specular_mul = max(0.0, dot(normal_vec, half_vec));
    }
    else
    {
        diffuse_mul = max(0.0, dot(-normal_vec, normalize(dir)));
        specular_mul = max(0.0, dot(-normal_vec, half_vec));
    }

    // calculate specular shine strength
    if(diffuse_mul <= 0.0001)
        specular_mul = 0.0;
    else
        specular_mul = pow(specular_mul, material.shininess) * base.strength;
}

void calc_point_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in Material material, in Point_light point_light, out vec3 scattered, out vec3 reflected)
{
    // point light location
    vec3 dir = point_light.pos_eye - pos;
    float dist = length(dir);

    dir = dir / dist; // normalize, but reuse length instead of calling normalize

    // calculate point light falloff
    float atten = 1.0 / (point_light.const_atten
        + point_light.linear_atten * dist
        + point_light.quad_atten * dist * dist);

    // midway between light and camera - for reflection calc
    vec3 half_vec = normalize(dir + forward);

    float diffuse_mul, specular_mul;
    calc_common_lighting(normal_vec, dir, half_vec, material, point_light.base,
        diffuse_mul, specular_mul);

    // diffuse light color
    scattered = point_light.base.color * diffuse_mul * atten;
    // specular light color
    reflected = point_light.base.color * specular_mul * atten;
}

void calc_dir_lighting(in vec3 normal_vec, in Material material, in Dir_light dir_light,
    out vec3 scattered, out vec3 reflected)
{
    float diffuse_mul, specular_mul;
    calc_common_lighting(normal_vec, dir_light.dir, dir_light.half_vec, material, dir_light.base,
        diffuse_mul, specular_mul);

    // diffuse light color
    scattered = dir_light.base.color * diffuse_mul;
    // specular light color
    reflected = dir_light.base.color * specular_mul;
}
