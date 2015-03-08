// gl_helpers.hpp
// common OpenGL utils

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

#ifndef GL_HELPERS_HPP
#define GL_HELPERS_HPP

#include <iostream>

#include <glm/glm.hpp>

void check_error(const char * at);

std::ostream & operator<<(std::ostream & out, const glm::vec2 & v);
std::ostream & operator<<(std::ostream & out, const glm::vec3 & v);
std::ostream & operator<<(std::ostream & out, const glm::vec4 & v);
std::ostream & operator<<(std::ostream & out, const glm::mat3 & m);
std::ostream & operator<<(std::ostream & out, const glm::mat4 & m);

#endif // GL_HELPERS_HPP
