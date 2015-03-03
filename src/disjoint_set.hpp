// disjoint_set.hpp
// disjoint set data struct (for Kruskal's Algorithm)

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

#ifndef DISJOINT_SET_HPP
#define DISJOINT_SET_HPP

#include <unordered_map>
#include <vector>

template <typename T>
class Disjoint_set final
{
public:
    Disjoint_set(const std::vector<T> & items);
    const T & find_rep(const T & a) const;
    void union_reps(const T & a, const T & b);
private:
    std::unordered_map<T, std::pair<T, unsigned int>> _set;
};

template <typename T>
Disjoint_set<T>::Disjoint_set(const std::vector<T> & items)
{
    for(const auto & i: items)
    {
        _set[i] = std::make_pair(i, 0);
    }
}

template <typename T>
const T & Disjoint_set<T>::find_rep(const T & a) const
{
    const T * x = &_set.at(a).first;

    if(a != *x)
        return find_rep(*x);
    else
        return a;
}

template <typename T>
void Disjoint_set<T>::union_reps(const T & a, const T & b)
{
    T a_root = find_rep(a);
    T b_root = find_rep(b);

    if(a_root == b_root)
        return;

    // compare ranks
    if(_set[a_root].second < _set[b_root].second)
        _set[a_root].first = b_root;
    else if(_set[a_root].second > _set[b_root].second)
        _set[b_root].first = a_root;
    else // equal ranks
    {
        _set[b_root].first = a_root;
        ++_set[a_root].second;
    }
}

#endif // DISJOINT_SET_HPP
