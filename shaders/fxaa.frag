// fxaa.frag
// FXAA frag shader

// based on code from Timothy Lottes: https://gist.github.com/bkaradzic/6011431

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

out vec4 frag_color;

uniform sampler2D scene_tex;
uniform vec2 rcp_viewport_size;


// FXAA parameters
const float edge_threshold = 0.166; // minimum contrast considered an edge
const float edge_threshold_min = 0.0833; // cuttoff to prevent processing darks
const float subpix_alias_removal = 0.75; // subpixel alias removal cap
// select a set of tuning parameters
#define FXAA_QUALITY_PRESET 12

/*============================================================================
                     FXAA QUALITY - MEDIUM DITHER PRESETS
============================================================================*/
#if (FXAA_QUALITY_PRESET == 10)
    const int quality_lvl = 3;
    const float quality_params[3] = float[3](1.5, 3.0, 12.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 11)
    const int quality_lvl = 4;
    const float quality_params[4] = float[4](1.0, 1.5, 3.0, 12.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 12)
    const int quality_lvl = 5;
    const float quality_params[5] = float[5](1.0, 1.5, 2.0, 4.0, 12.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 13)
    const int quality_lvl = 6;
    const float quality_params[6] = float[6](1.0, 1.5, 2.0, 2.0, 4.0, 12.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 14)
    const int quality_lvl = 7;
    const float quality_params[7] = float[7](1.0, 1.5, 2.0, 2.0, 2.0, 4.0, 12.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 15)
    const int quality_lvl = 8;
    const float quality_params[8] = float[8](1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 12.0);
#endif

/*============================================================================
                     FXAA QUALITY - LOW DITHER PRESETS
============================================================================*/
#if (FXAA_QUALITY_PRESET == 20)
    const int quality_lvl = 3;
    const float quality_params[3] = float[3](1.5, 2.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 21)
    const int quality_lvl = 4;
    const float quality_params[4] = float[4](1.0, 1.5, 2.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 22)
    const int quality_lvl = 5;
    const float quality_params[5] = float[5](1.0, 1.5, 2.0, 2.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 23)
    const int quality_lvl = 6;
    const float quality_params[6] = float[6](1.0, 1.5, 2.0, 2.0, 2.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 24)
    const int quality_lvl = 7;
    const float quality_params[7] = float[7](1.0, 1.5, 2.0, 2.0, 2.0, 3.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 25)
    const int quality_lvl = 8;
    const float quality_params[8] = float[8](1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 26)
    const int quality_lvl = 9;
    const float quality_params[9] = float[9](1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 27)
    const int quality_lvl = 10;
    float quality_params[10] = float[10](1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 28)
    const int quality_lvl = 11;
    const float quality_params[11] = float[11](1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_QUALITY_PRESET == 29)
    const int quality_lvl = 12;
    const float quality_params[12] = float[12](1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);
#endif

/*============================================================================
                     FXAA QUALITY - EXTREME QUALITY
============================================================================*/
#if (FXAA_QUALITY_PRESET == 39)
    const int quality_lvl = 12;
    const float quality_params[12] = float[12](1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);
#endif

// TODO: document
void main()
{
    vec2 pos_m = gl_FragCoord.xy * rcp_viewport_size;

    // get current pixel
    vec4 rgb_lum_m = textureLod(scene_tex, pos_m, 0.0);
    float luma_m = rgb_lum_m.a;

    // get neighboring pixels luma
    float luma_s = textureLodOffset(scene_tex, pos_m, 0.0, ivec2( 0,  1)).a;
    float luma_e = textureLodOffset(scene_tex, pos_m, 0.0, ivec2( 1,  0)).a;
    float luma_n = textureLodOffset(scene_tex, pos_m, 0.0, ivec2( 0, -1)).a;
    float luma_w = textureLodOffset(scene_tex, pos_m, 0.0, ivec2(-1,  0)).a;

    // get max and minimum luma
    float range_max = max(max(luma_n, luma_w), max(luma_e, max(luma_s, luma_m)));
    float range_min = min(min(luma_n, luma_w), min(luma_e, min(luma_s, luma_m)));

    // calculate luma range (edge detection)
    float range = range_max - range_min;
    float range_max_clamped = max(edge_threshold_min, range * edge_threshold);

    // if range is below max (no edge), we don't need to AA this pixel - use prev val & exit
    if(range < range_max_clamped)
    {
        frag_color = rgb_lum_m;
        return;
    }

    // get diagonal neighbor pixels for subpixel alias test
    float luma_nw = textureLodOffset(scene_tex, pos_m, 0.0, ivec2(-1, -1)).a;
    float luma_se = textureLodOffset(scene_tex, pos_m, 0.0, ivec2( 1,  1)).a;
    float luma_ne = textureLodOffset(scene_tex, pos_m, 0.0, ivec2( 1, -1)).a;
    float luma_sw = textureLodOffset(scene_tex, pos_m, 0.0, ivec2(-1,  1)).a;

    float luma_ns = luma_n + luma_s;
    float luma_we = luma_w + luma_e;

    float luma_nese = luma_ne + luma_se;
    float luma_nwne = luma_nw + luma_ne;
    float luma_nwsw = luma_nw + luma_sw;
    float luma_swse = luma_sw + luma_se;

    // detect if edge is vertical or horizontal
    float edge_horz = abs((-2.0 * luma_w) + luma_nwsw) + (abs((-2.0 * luma_m) + luma_ns) * 2.0) + abs((-2.0 * luma_e) + luma_nese);
    float edge_vert = abs((-2.0 * luma_s) + luma_swse) + (abs((-2.0 * luma_m) + luma_we) * 2.0) + abs((-2.0 * luma_n) + luma_nwne);
    bool horz_span = edge_horz >= edge_vert;

    float length_sign = rcp_viewport_size.x; // sign of gradient

    if(!horz_span)
    {
        luma_n = luma_w;
        luma_s = luma_e;
    }
    else
    {
        length_sign = rcp_viewport_size.y;
    }

    float luma_nn = luma_n + luma_m;
    float luma_ss = luma_s + luma_m;

    // calc luminance gradient
    float gradient_n = luma_n - luma_m;
    float gradient_s = luma_s - luma_m;
    float gradient = max(abs(gradient_n), abs(gradient_s));
    bool pair_n = abs(gradient_n) >= abs(gradient_s);

    if(pair_n)
    {
        length_sign = -length_sign;
    }
    else
    {
        luma_nn = luma_ss;
    }

    // calc pixel pos in direction of gradient
    vec2 pos_b = pos_m;

    if(!horz_span)
    {
        pos_b.x += length_sign * 0.5;
    }
    else
    {
        pos_b.y += length_sign * 0.5;
    }

    // offset for TODO
    vec2 off_n_p = vec2(
        (!horz_span) ? 0.0 : rcp_viewport_size.x,
        (horz_span) ? 0.0 : rcp_viewport_size.y);

    // pos of offset pixels n & p
    vec2 pos_n = pos_b - off_n_p * quality_params[0];
    vec2 pos_p = pos_b + off_n_p * quality_params[0];

    float luma_mm = luma_m - luma_nn * 0.5;
    bool luma_m_lt_zero = luma_mm < 0.0;

    float luma_end_n = textureLod(scene_tex, pos_n, 0.0).a - luma_nn * 0.5;
    float luma_end_p = textureLod(scene_tex, pos_p, 0.0).a - luma_nn * 0.5;

    float gradient_scaled = gradient * 1.0 / 4.0;

    // tweak offsets based on gradient scale
    bool done_n = abs(luma_end_n) >= gradient_scaled;
    if(!done_n)
    {
        pos_n.x -= off_n_p.x * quality_params[1];
    }
    else
    {
        pos_n.y -= off_n_p.y * quality_params[1];
    }

    bool done_p = abs(luma_end_p) >= gradient_scaled;
    if(!done_p)
    {
        pos_p.x += off_n_p.x * quality_params[1];
    }
    else
    {
        pos_p.y += off_n_p.y * quality_params[1];
    }

    // continue to tweak offsets until quality level is met or endpoints >= gradient
    int i = 2;
    while(!(done_n && done_p) && i < quality_lvl)
    {
        if(!done_n)
        {
            luma_end_n = textureLod(scene_tex, pos_n.xy, 0.0).a;
            luma_end_n = luma_end_n - luma_nn * 0.5;
        }
        if(!done_p)
        {
            luma_end_p = textureLod(scene_tex, pos_p.xy, 0.0).a;
            luma_end_p = luma_end_p - luma_nn * 0.5;
        }

        done_n = abs(luma_end_n) >= gradient_scaled;
        done_p = abs(luma_end_p) >= gradient_scaled;

        if(!done_n)
        {
            pos_n.x -= off_n_p.x * quality_params[i];
            pos_n.y -= off_n_p.y * quality_params[i];
        }
        if(!done_p)
        {
            pos_p.x += off_n_p.x * quality_params[i];
            pos_p.y += off_n_p.y * quality_params[i];
        }

        ++i;
    }

    // offset distance
    float dst_n = pos_m.x - pos_n.x;
    float dst_p = pos_p.x - pos_m.x;

    if(!horz_span)
    {
        dst_n = pos_m.y - pos_n.y;
        dst_p = pos_p.y - pos_m.y;
    }

    float dst = min(dst_n, dst_p);

    float span_length_rcp = 1.0 / (dst_p + dst_n);

    bool good_span_n = (luma_end_n < 0.0) != luma_m_lt_zero;
    bool good_span_p = (luma_end_p < 0.0) != luma_m_lt_zero;
    bool good_span = dst_n < dst_p ? good_span_n : good_span_p;

    float subpix_nswe = luma_ns + luma_we;
    float subpix_nwswnese = luma_nwsw + luma_nese;

    // calculate subpixel offset factor
    float subpix_a = subpix_nswe * 2.0 + subpix_nwswnese;
    float subpix_b = (subpix_a * (1.0 / 12.0)) - luma_m;
    float subpix_c = clamp(abs(subpix_b) * 1.0 / range, 0.0, 1.0);
    float subpix_d = ((-2.0) * subpix_c) + 3.0;
    float subpix_e = subpix_c * subpix_c;
    float subpix_f = subpix_d * subpix_e;
    float subpix_g = subpix_f * subpix_f;
    float subpix_h = subpix_g * subpix_alias_removal;

    // get final offset
    float pixel_offset = (dst * (-span_length_rcp)) + 0.5;
    float pixel_offset_good = good_span ? pixel_offset : 0.0;
    float pixel_offset_subpix = max(pixel_offset_good, subpix_h);

    // get final pix pos
    if(!horz_span)
    {
        pos_m.x += pixel_offset_subpix * length_sign;
    }
    else
    {
        pos_m.y += pixel_offset_subpix * length_sign;
    }

    // return offset pixel color
    frag_color = vec4(textureLod(scene_tex, pos_m, 0.0).rgb, luma_m);
}
