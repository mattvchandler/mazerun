// spot_light.frag
// spot light pass

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

void calc_spot_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in float shininess, in Spot_light spot_light,
    out vec3 diffuse, out vec3 specular);

vec3 calc_view_pos(in vec2 map_coords, in sampler2D depth_map, in mat4 proj_mat,
    in vec2 view_ray);

in vec2 view_ray;

uniform mat4 proj_mat;

uniform Spot_light spot_light;

uniform sampler2D norm_shininess_map;
uniform sampler2D depth_map;
uniform vec2 viewport_size;

// camera facing direction (always (0, 0, 1) when viewed from camera)
uniform vec3 cam_light_forward; // TODO: set as const?

out vec4 diffuse;
out vec4 specular;

void main()
{
    vec2 map_coords = gl_FragCoord.xy / viewport_size;
    vec3 pos = calc_view_pos(map_coords, depth_map, proj_mat, view_ray);
    vec4 norm_shininess = texture(norm_shininess_map, map_coords);
    float shininess = norm_shininess.w;
    vec3 normal_vec = norm_shininess.xyz;

    vec3 diffuse_tmp, specular_tmp;

    calc_spot_lighting(pos, cam_light_forward, normal_vec, shininess, spot_light,
        diffuse_tmp, specular_tmp);

    diffuse = vec4(diffuse_tmp, 1.0);
    specular =vec4(specular_tmp, 1.0);
}
