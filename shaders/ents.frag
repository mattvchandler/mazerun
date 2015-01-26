// ents.frag
// fragment shader for most entities

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

struct Material
{
    vec3 specular;
    float shininess;
};

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

void calc_point_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in Material material, in Point_light point_light, out vec3 scattered, out vec3 reflected);

void calc_dir_lighting(in vec3 normal_vec, in Material material, in Dir_light dir_light,
    out vec3 scattered, out vec3 reflected);

in vec2 tex_coords;
in vec3 pos;
in vec3 normal_vec;

// material vars
uniform sampler2D tex;
uniform sampler2D norm_map;
uniform Material material;

// lighting vars
uniform vec3 ambient_color;
uniform Dir_light dir_light;

// camera facing direction (always (0, 0, 1) when viewed from camera)
uniform vec3 cam_light_forward;

out vec4 frag_color;

void main()
{
    vec3 scattered = ambient_color, reflected = vec3(0.0, 0.0, 0.0);
    vec3 tmp_scattered, tmp_reflected;

    // calc_point_lighting(pos, light_forward, normal_vec, material, cam_light,
    //     tmp_scattered, tmp_reflected);
    // scattered += tmp_scattered;
    // reflected += tmp_reflected;

    calc_dir_lighting(normal_vec, material, dir_light, tmp_scattered, tmp_reflected);
    scattered += tmp_scattered;
    reflected += tmp_reflected;

    // add to material color (from texture) to lighting for final color
    vec3 rgb = min(texture(tex, tex_coords).rgb * scattered + reflected, vec3(1.0));
    frag_color = vec4(rgb, 1.0);
}
