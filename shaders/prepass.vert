// prepass.vert
// vertex shader for scene prepass

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

in vec3 vert_pos;
in vec2 vert_tex_coords;
in vec3 vert_normals;
in vec3 vert_tangents;

uniform mat4 model_view_proj;
uniform mat4 model_view;
uniform mat3 normal_transform;

out vec3 pos;
out vec2 tex_coord;
out vec3 normal_vec;
out vec3 tangent;

void main()
{
    tex_coord = vert_tex_coords;

    // transform the into view space coordinates
    pos = vec3(model_view * vec4(vert_pos, 1.0));
    normal_vec = normalize(normal_transform * vert_normals);
    tangent = normalize(normal_transform * vert_tangents);

    gl_Position = model_view_proj * vec4(vert_pos, 1.0);
}