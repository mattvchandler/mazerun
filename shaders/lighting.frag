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
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    float shininess;
    vec3 emissive_color;
    // float reflectivity;

    sampler2D ambient_map;
    sampler2D diffuse_map;
    sampler2D specular_map;
    sampler2D shininess_map; // greyscale
    sampler2D emissive_map;
    // sampler2D reflectivity_map; // greyscale
    sampler2D normal_map;
};

// lighting vars
struct Base_light
{
    vec3 color;
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
    mat4 shadow_mat;
};

struct Dir_light
{
    Base_light base;
    vec3 dir;
    vec3 half_vec;
};

void calc_common_lighting(in vec3 normal_vec, in vec3 dir, in vec3 half_vec,
    in float shininess, in vec3 color, in float atten,
    out vec3 diffuse, out vec3 specular)
{
    // calculate ammt of diffuse and specular shading
    float diffuse_mul, specular_mul;
    diffuse_mul = max(0.0, dot(normal_vec, dir));
    specular_mul = max(0.0, dot(normal_vec, half_vec));

    // calculate specular shine strength
    if(diffuse_mul <= 0.0001)
        specular_mul = 0.0;
    else
        specular_mul = pow(specular_mul, shininess);

    // diffuse light color
    diffuse = color * diffuse_mul * atten;
    // specular light color
    specular = color * specular_mul * atten;
}

float calc_attenuation(in float const_atten, in float linear_atten, in float quad_atten, in float dist)
{
    return  1.0 / (const_atten +
        linear_atten * dist +
        quad_atten * dist * dist);
}

void calc_dist_dir(in vec3 light_pos_eye, in vec3 pos, out vec3 dir, out float dist)
{
    dir = light_pos_eye - pos;
    dist = length(dir);

    dir = dir / dist; // normalize, but reuse length instead of calling normalize
}

void calc_point_spot_light(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in vec3 dir, in float atten, in float shininess, in vec3 color,
    out vec3 diffuse, out vec3 specular)
{
    // midway between light and camera - for reflection calc
    vec3 half_vec = normalize(dir + forward);

    calc_common_lighting(normal_vec, dir, half_vec, shininess, color, atten,
        diffuse, specular);
}

void calc_point_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in float shininess, in Point_light point_light,
    out vec3 diffuse, out vec3 specular)
{
    vec3 dir;
    float dist;
    calc_dist_dir(point_light.pos_eye, pos, dir, dist);

    // calculate light falloff
    float atten = calc_attenuation(point_light.const_atten, point_light.linear_atten,
        point_light.quad_atten, dist);

    calc_point_spot_light(pos, forward, normal_vec, dir, atten, shininess,
        point_light.base.color, diffuse, specular);
}

void calc_spot_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in float shininess, in Spot_light spot_light,
    out vec3 diffuse, out vec3 specular)
{
    vec3 dir;
    float dist;
    calc_dist_dir(spot_light.pos_eye, pos, dir, dist);

    // calculate light falloff
    float atten = calc_attenuation(spot_light.const_atten, spot_light.linear_atten,
        spot_light.quad_atten, dist);

    float spot_cos = dot(dir, -spot_light.dir_eye);

    if(spot_cos < spot_light.cos_cutoff)
        atten = 0.0;
    else
        atten *= pow(spot_cos, spot_light.exponent);

    calc_point_spot_light(pos, forward, normal_vec, dir, atten, shininess,
        spot_light.base.color, diffuse, specular);
}

void calc_dir_lighting(in vec3 normal_vec, in float shininess,
    in Dir_light dir_light, out vec3 diffuse, out vec3 specular)
{
    calc_common_lighting(normal_vec, dir_light.dir, dir_light.half_vec, shininess,
        dir_light.base.color, 1.0, diffuse, specular);
}
