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
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    vec3 emissive_color;
    // float reflectivity;

    sampler2D ambient_map;
    sampler2D diffuse_map;
    sampler2D specular_map;
    sampler2D emissive_map;
    // sampler2D reflectivity_map; // greyscale
};

in vec2 tex_coord;

// material vars
uniform Material material;

uniform vec3 ambient_light_color;

uniform sampler2D diffuse_fbo_tex;
uniform sampler2D specular_fbo_tex;
uniform vec2 viewport_size;

out vec4 frag_color;

void main()
{
    vec2 map_coords = gl_FragCoord.xy / viewport_size;

    vec3 diffuse = material.ambient_color * texture(material.ambient_map, tex_coord).rgb * ambient_light_color +
        texture(diffuse_fbo_tex, map_coords).rgb;
    vec3 specular = texture(specular_fbo_tex, map_coords).rgb;

    // add to material color (from textures) to lighting for final color
    vec3 rgb = min(material.emissive_color * texture(material.emissive_map, tex_coord).rgb +
        material.diffuse_color * texture(material.diffuse_map, tex_coord).rgb * diffuse +
        material.specular_color * texture(material.specular_map, tex_coord).rgb * specular, vec3(1.0));
    frag_color = vec4(rgb, 1.0);
}
