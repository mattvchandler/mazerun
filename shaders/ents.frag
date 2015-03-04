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
    vec3 specular_color;
    float shininess;
    sampler2D diffuse_map;
    sampler2D normal_map;
    sampler2D emissive_map;
};

struct Base_light
{
    bool enabled;
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

struct Spot_light
{
    Base_light base;
    vec3 pos_eye;
    vec3 dir_eye;
    float cos_cutoff;
    float exponent;
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

vec3 norm_map_normal(in vec2 tex_coord, in vec3 normal, in vec3 tangent, in sampler2D normal_map);

void calc_point_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in Material material, in Point_light point_light, out vec3 scattered, out vec3 reflected);

void calc_spot_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in Material material, in Spot_light spot_light, out vec3 scattered, out vec3 reflected);

void calc_dir_lighting(in vec3 normal_vec, in Material material, in Dir_light dir_light,
    out vec3 scattered, out vec3 reflected);

in vec3 pos;
in vec2 tex_coord;
in vec3 normal_vec;
in vec3 tangent;

// material vars
uniform Material material;

// lighting vars
uniform vec3 ambient_color;
const int max_point_lights = 10; // TODO: set by config?
const int max_spot_lights = 10; // TODO: set by config?
uniform int num_point_lights;
uniform int num_spot_lights;
uniform Point_light point_lights[max_point_lights];
uniform Spot_light spot_lights[max_spot_lights];
uniform Dir_light dir_light;

// camera facing direction (always (0, 0, 1) when viewed from camera)
uniform vec3 cam_light_forward;

out vec4 frag_color;

void main()
{
    vec3 scattered = ambient_color, reflected = vec3(0.0);
    vec3 tmp_scattered, tmp_reflected;

    vec3 mapped_normal = norm_map_normal(tex_coord, normal_vec, tangent, material.normal_map);

    for(int i = 0; i < num_point_lights; ++i)
    {
        calc_point_lighting(pos, cam_light_forward, mapped_normal, material, point_lights[i],
                tmp_scattered, tmp_reflected);
        scattered += tmp_scattered;
        reflected += tmp_reflected;
    }

    for(int i = 0; i < num_spot_lights; ++i)
    {
        calc_spot_lighting(pos, cam_light_forward, mapped_normal, material, spot_lights[i],
                tmp_scattered, tmp_reflected);
        scattered += tmp_scattered;
        reflected += tmp_reflected;
    }

    calc_dir_lighting(mapped_normal, material, dir_light, tmp_scattered, tmp_reflected);
    scattered += tmp_scattered;
    reflected += tmp_reflected;

    // add to material color (from texture) to lighting for final color
    vec3 rgb = min(texture(material.emissive_map, tex_coord).rgb + texture(material.diffuse_map, tex_coord).rgb * scattered
        + material.specular_color * reflected, vec3(1.0));
    frag_color = vec4(rgb, 1.0);
}
