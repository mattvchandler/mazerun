// ents.vert
// vertex shader for most entites

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

struct Base_light
{
    bool enabled;
    vec3 color;
    bool casts_shadow;
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

in vec3 vert_pos;
in vec2 vert_tex_coords;
in vec3 vert_normals;
in vec3 vert_tangents;

uniform mat4 model_view_proj;
uniform mat4 model_view;
uniform mat4 model;
uniform mat3 normal_transform;

// lighting uniforms
const int max_point_lights = 10; // TODO: set by config?
const int max_spot_lights = 10; // TODO: set by config?
uniform int num_point_lights;
uniform int num_spot_lights;
uniform Point_light point_lights[max_point_lights];
uniform Spot_light spot_lights[max_spot_lights];

out vec3 pos;
out vec2 tex_coord;
out vec3 normal_vec;
out vec3 tangent;

// shadow outputs
out vec4 spot_shadow_coords[max_spot_lights];

void main()
{
    tex_coord = vert_tex_coords;

    // transform the into view space coordinates
    pos = vec3(model_view * vec4(vert_pos, 1.0));
    normal_vec = normalize(normal_transform * vert_normals);
    tangent = normalize(normal_transform * vert_tangents);

    for(int i = 0; i < num_spot_lights && i < 2; ++i) // TODO: work around texture limits
    {
        if(spot_lights[i].base.casts_shadow)
            spot_shadow_coords[i] = spot_lights[i].shadow_mat * model * vec4(vert_pos, 1.0);
    }

    gl_Position = model_view_proj * vec4(vert_pos, 1.0);
}
