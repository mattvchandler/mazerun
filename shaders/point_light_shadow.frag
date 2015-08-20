// point_light_shadow.frag
// point light pass w/ shadows

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

struct Point_light
{
    Base_light base;
    vec3 pos_eye;
    float const_atten;
    float linear_atten;
    float quad_atten;
};

void calc_point_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in float shininess, in Point_light point_light,
    out vec3 diffuse, out vec3 specular);

vec3 calc_view_pos(in vec2 map_coords, in sampler2D depth_map, in mat4 proj_mat,
    in vec2 view_ray);

in vec2 view_ray;

uniform mat4 proj_mat;

uniform Point_light point_light;

uniform sampler2D normal_shininess_map;
uniform sampler2D depth_map;
uniform vec2 rcp_viewport_size;

// camera facing direction (always (0, 0, 1) when viewed from camera)
uniform vec3 cam_light_forward; // TODO: set as const?

uniform samplerCube shadow_map;
uniform vec3 light_world_pos;
uniform mat4 inv_cam_view;

out vec4 diffuse;
out vec4 specular;

void main()
{
    vec2 map_coords = gl_FragCoord.xy * rcp_viewport_size;
    vec3 pos = calc_view_pos(map_coords, depth_map, proj_mat, view_ray);
    vec4 norm_shininess = texture(normal_shininess_map, map_coords);
    float shininess = norm_shininess.w;
    vec3 normal_vec = norm_shininess.xyz;

    vec3 diffuse_tmp, specular_tmp;

    calc_point_lighting(pos, cam_light_forward, normal_vec, shininess, point_light,
        diffuse_tmp, specular_tmp);

    vec3 world_pos = vec3(inv_cam_view * vec4(pos, 1.0));
    vec3 shadow_dir = world_pos - light_world_pos;
    float shadow_dist = length(shadow_dir);
    shadow_dir /= shadow_dist;

    float shadow_ref_dist = texture(shadow_map, shadow_dir).r;

    float shadow = 0.0;
    if(shadow_ref_dist > shadow_dist - 0.001)
        shadow = 1.0;
    else
        shadow = 0.0;

    diffuse = shadow * vec4(diffuse_tmp, 1.0);
    specular = shadow * vec4(specular_tmp, 1.0);
}
