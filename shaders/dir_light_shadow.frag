// dir_light_shadow.frag
// directional light pass w/ shadows

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

// lighting vars
struct Base_light
{
    vec3 color;
};

struct Dir_light
{
    Base_light base;
    vec3 dir;
    vec3 half_vec;
};

void calc_dir_lighting(in vec3 normal_vec, in float shininess,
    in Dir_light dir_light, out vec3 diffuse, out vec3 specular);

uniform Dir_light dir_light;

uniform sampler2D pos_map;
uniform sampler2D shininess_map;
uniform sampler2D normal_map;
uniform vec2 viewport_size;

uniform mat4 shadow_mat;
uniform sampler2DShadow shadow_map;

out vec4 diffuse;
out vec4 specular;

void main()
{
    vec2 map_coords = gl_FragCoord.xy / viewport_size;
    vec3 pos = texture(pos_map, map_coords).xyz;
    float shininess = texture(shininess_map, map_coords).x;
    vec3 normal_vec = texture(normal_map, map_coords).xyz;

    float shadow = textureProj(shadow_map, shadow_mat * vec4(pos, 1.0)); // not too happy about per-pixel mat mult

    vec3 diffuse_tmp, specular_tmp;

    calc_dir_lighting(normal_vec, shininess, dir_light,
        diffuse_tmp, specular_tmp);

    diffuse = shadow * vec4(diffuse_tmp, 1.0);
    specular = shadow * vec4(specular_tmp, 1.0);
}
