// main.cpp
// main entry point

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

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <random>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

// global prng
std::random_device rng;
std::mt19937 prng;

enum Direction {UP = 0, DOWN, LEFT, RIGHT};

template <typename T>
class Disjoint_set
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

// make sf::Vector2u hashable
namespace std
{
    template <>
    struct hash<sf::Vector2u>
    {
    public:
        size_t operator()(const sf::Vector2u & a) const
        {
            return hash<unsigned int>()(a.x) ^ hash<unsigned int>()(a.y);
        }
    };
};

struct Wall
{
    sf::Vector2u cell_1, cell_2;
    Direction dir_1, dir_2;
    int region_1, region_2;
};

class Grid_cell
{
public:
    Grid_cell();
    bool walls[4];
    bool visited;
    int region;
    bool room;
};

Grid_cell::Grid_cell(): visited(false), region(-1), room(false)
{
    for(int i = 0; i < 4; ++i)
        walls[i] = true;
}

class Maze_grid
{
public:
    Maze_grid(const sf::Vector2u & grid_size);
    void init();

    std::vector<std::vector<Grid_cell>> grid;
private:
    void gen_rooms(const std::function<void(Maze_grid &, const sf::Vector2u &, const int)> & mazegen,
        const unsigned int room_attempts, const unsigned int wall_rm_attempts);
    void mazegen_dfs(const sf::Vector2u & start, const int region);
    void mazegen_prim(const sf::Vector2u & start, const int region);
    void mazegen_kruskal(const sf::Vector2u & start, const int region);
};

Maze_grid::Maze_grid(const sf::Vector2u & grid_size)
    :grid(grid_size.y, std::vector<Grid_cell>(grid_size.x))
{
    if(grid_size.y == 0)
    {
        throw std::invalid_argument("grid_size.y == 0");
    }
    if(grid_size.x == 0)
    {
        throw std::invalid_argument("grid_size.x == 0");
    }
}

void Maze_grid::init()
{
    gen_rooms(&Maze_grid::mazegen_dfs, 25, 100);
}

void Maze_grid::gen_rooms(const std::function<void(Maze_grid &, const sf::Vector2u &, const int)> & mazegen,
    const unsigned int room_attempts, const unsigned int wall_rm_attempts)
{
    int region = 0;
    // place some random rooms
    for(unsigned int i = 0; i  < room_attempts; ++i)
    {
        sf::Vector2u size(std::binomial_distribution<unsigned int>(std::min(9u, (unsigned int)(grid[0].size() - 1)), 0.5)(prng) + 1,
           std::binomial_distribution<unsigned int>(std::min(9u, (unsigned int)(grid.size() - 1)), 0.5)(prng) + 1);

        // discard skinny rooms
        if((float)size.x / (float)size.y > 4 || (float)size.y / (float)size.x > 4)
            continue;

        sf::Vector2u pos(std::uniform_int_distribution<unsigned int>(0, std::max(0, (int)grid[0].size() - (int)size.x - 1))(prng),
           std::uniform_int_distribution<unsigned int>(0, std::max(0, (int)grid.size() - (int)size.y - 1))(prng));

        // std::cout<<"pos:("<<pos.x<<","<<pos.y<<") size:("<<size.x<<","<<size.y<<")"<<std::endl;

        // check if overlapping
        bool overlapped = false;
        for(size_t row = pos.y; row < pos.y +size.y; ++row)
        {
            for(size_t col = pos.x; col < pos.x +size.x; ++col)
            {
                if(grid[row][col].visited)
                {
                    overlapped = true;
                    goto overlap;
                }
            }
        }
        overlap:
        // discard if overlapping
        if(overlapped)
            continue;

        // mark visited
        // destroy walls, excepting borders
        for(size_t row = pos.y; row < pos.y + size.y; ++row)
        {
            for(size_t col = pos.x; col < pos.x + size.x; ++col)
            {
                grid[row][col].visited = true;
                grid[row][col].region = region;
                grid[row][col].room = true;

                if(row > pos.y)
                    grid[row][col].walls[UP] = false;
                if(row < pos.y + size.y - 1)
                    grid[row][col].walls[DOWN] = false;
                if(col > pos.x)
                    grid[row][col].walls[LEFT] = false;
                if(col < pos.x + size.x - 1)
                    grid[row][col].walls[RIGHT] = false;
            }
        }
        ++region;
    }

    // fill in remaining areas with mazes
    for(size_t row = 0; row < grid.size(); ++row)
    {
        for(size_t col = 0; col < grid[0].size(); ++col)
        {
            if(!grid[row][col].visited)
            {
                mazegen(*this, sf::Vector2u(col, row), region);
                ++region;
            }
        }
    }

    // find connectors, build graph
    std::vector<Wall> connectors;

    for(size_t row = 0; row < grid.size() - 1; ++row)
    {
        for(size_t col = 0; col < grid[0].size() - 1; ++col)
        {
            if(grid[row][col].region != grid[row][col + 1].region)
            {
                sf::Vector2u cell_1(col, row);
                sf::Vector2u cell_2(col + 1, row);
                int region_1 = grid[row][col].region;
                int region_2 = grid[row][col + 1].region;

                connectors.push_back({cell_1, cell_2, RIGHT, LEFT, region_1, region_2});
            }
            if(grid[row][col].region != grid[row + 1][col].region)
            {
                sf::Vector2u cell_1(col, row);
                sf::Vector2u cell_2(col, row + 1);
                int region_1 = grid[row][col].region;
                int region_2 = grid[row + 1][col].region;

                connectors.push_back({cell_1, cell_2, DOWN, UP, region_1, region_2});
            }
        }
    }

    // shuffle connectors
    std::shuffle(connectors.begin(), connectors.end(), prng);

    // use kruskals alg to find min spanning tree of region graph
    std::vector<int> regions_vec(region);
    for(size_t i = 0; i < regions_vec.size(); ++i)
        regions_vec[i] = i;

    Disjoint_set<int>regions(regions_vec);

    int sets = region;
    for(const auto & conn: connectors)
    {
        int set_1 = regions.find_rep(conn.region_1);
        int set_2 = regions.find_rep(conn.region_2);

        if(set_1 != set_2)
        {
            regions.union_reps(set_1, set_2);

            // destroy walls joining regions
            grid[conn.cell_1.y][conn.cell_1.x].walls[conn.dir_1] = false;
            grid[conn.cell_2.y][conn.cell_2.x].walls[conn.dir_2] = false;

            if(--sets == 1)
                break;
        }
    }

    //  randomly destroy random walls
    for(unsigned int i = 0; i  < wall_rm_attempts; ++i)
    {
        sf::Vector2u cell(std::uniform_int_distribution<unsigned int>(0, grid[0].size() - 1)(prng),
            std::uniform_int_distribution<unsigned int>(0, grid[0].size() - 1)(prng));

        Direction wall = (Direction)std::uniform_int_distribution<int>(0, 3)(prng);

        switch(wall)
        {
        case UP:
            if(cell.y > 0)
            {
                grid[cell.y][cell.x].walls[UP] = false;
                grid[cell.y - 1][cell.x].walls[DOWN] = false;
            }
            break;
        case DOWN:
            if(cell.y < grid.size() - 1)
            {
                grid[cell.y][cell.x].walls[DOWN] = false;
                grid[cell.y + 1][cell.x].walls[UP] = false;
            }
            break;
        case LEFT:
            if(cell.x > 0)
            {
                grid[cell.y][cell.x].walls[LEFT] = false;
                grid[cell.y][cell.x - 1].walls[RIGHT] = false;
            }
            break;
        case RIGHT:
            if(cell.x < grid[0].size() - 1)
            {
                grid[cell.y][cell.x].walls[RIGHT] = false;
                grid[cell.y][cell.x + 1].walls[LEFT] = false;
            }
            break;
        }
    }
}

void Maze_grid::mazegen_dfs(const sf::Vector2u & start, const int region)
{
    if(grid[start.y][start.x].visited)
        return;

    // to avoid stack overflow, maintain our own heap-allocated stack instead of
    //  recursing
    struct State
    {
        sf::Vector2u pos;
        Direction dirs[4];
        unsigned short dir_i;
    };
    std::vector<State> state_stack;

    state_stack.push_back({start, {UP, DOWN, LEFT, RIGHT}, 0});
    std::shuffle(std::begin(state_stack.back().dirs), std::end(state_stack.back().dirs), prng);

    grid[start.y][start.x].visited = true;
    grid[start.y][start.x].region = region;

    while(state_stack.size() > 0)
    {
        State & state = state_stack.back();
        if(state.dir_i >= 4)
        {
            state_stack.pop_back();
            continue;
        }

        sf::Vector2u next;
        bool found_next = false;

        switch(state.dirs[state.dir_i++])
        {
        case UP:
            if(state.pos.y > 0 && !grid[state.pos.y - 1][state.pos.x].visited)
            {
                next = sf::Vector2u(state.pos.x, state.pos.y - 1);
                grid[state.pos.y][state.pos.x].walls[UP] = false;
                grid[next.y][next.x].walls[DOWN] = false;
                found_next = true;
            }
            break;
        case DOWN:
            if(state.pos.y < grid.size() - 1 && !grid[state.pos.y + 1][state.pos.x].visited)
            {
                next = sf::Vector2u(state.pos.x, state.pos.y + 1);
                grid[state.pos.y][state.pos.x].walls[DOWN] = false;
                grid[next.y][next.x].walls[UP] = false;
                found_next = true;
            }
            break;
        case LEFT:
            if(state.pos.x > 0 && !grid[state.pos.y][state.pos.x - 1].visited)
            {
                next = sf::Vector2u(state.pos.x - 1, state.pos.y);
                grid[state.pos.y][state.pos.x].walls[LEFT] = false;
                grid[next.y][next.x].walls[RIGHT] = false;
                found_next = true;
            }
            break;
        case RIGHT:
            if(state.pos.x < grid[state.pos.y].size() - 1 && !grid[state.pos.y][state.pos.x + 1].visited)
            {
                next = sf::Vector2u(state.pos.x + 1, state.pos.y);
                grid[state.pos.y][state.pos.x].walls[RIGHT] = false;
                grid[next.y][next.x].walls[LEFT] = false;
                found_next = true;
            }
            break;
        }

        if(found_next)
        {
            grid[next.y][next.x].visited = true;
            grid[next.y][next.x].region = region;
            state_stack.push_back({next, {UP, DOWN, LEFT, RIGHT}, 0});
            std::shuffle(std::begin(state_stack.back().dirs), std::end(state_stack.back().dirs), prng);
        }
    }
}

void Maze_grid::mazegen_prim(const sf::Vector2u & start, const int region)
{
    if(grid[start.y][start.x].visited)
        return;

    std::vector<Wall> walls;

    auto add_cell = [& walls, this](const sf::Vector2u & cell)
    {
        for(int i = 0; i < 4; ++i)
        {
            switch(i)
            {
            case UP:
                if(cell.y > 0 && !grid[cell.y - 1][cell.x].visited)
                {
                    walls.push_back({cell, sf::Vector2u(cell.x, cell.y - 1), UP, DOWN});
                }
                break;
            case DOWN:
                if(cell.y < grid.size() - 1 && !grid[cell.y + 1][cell.x].visited)
                {
                    walls.push_back({cell, sf::Vector2u(cell.x, cell.y + 1), DOWN, UP});
                }
                break;
            case LEFT:
                if(cell.x > 0 && !grid[cell.y][cell.x - 1].visited)
                {
                    walls.push_back({cell, sf::Vector2u(cell.x - 1, cell.y), LEFT, RIGHT});
                }
                break;
            case RIGHT:
                if(cell.x < grid[cell.y].size() - 1 && !grid[cell.y][cell.x + 1].visited)
                {
                    walls.push_back({cell, sf::Vector2u(cell.x + 1, cell.y), RIGHT, LEFT});
                }
                break;
            }
        }
    };

    add_cell(start);

    grid[start.y][start.x].visited = true;
    grid[start.y][start.x].region = region;

    while(walls.size() > 0)
    {
        std::uniform_int_distribution<size_t> rand_wall(0, walls.size() - 1);
        size_t curr_ind = rand_wall(prng);
        auto curr_it = walls.begin() + curr_ind;

        sf::Vector2u curr = curr_it->cell_1;
        Direction curr_dir = curr_it->dir_1;

        sf::Vector2u next = curr_it->cell_2;
        Direction next_dir = curr_it->dir_2;

        walls.erase(curr_it);

        if(!grid[next.y][next.x].visited)
        {
            grid[next.y][next.x].visited = true;
            grid[next.y][next.x].region = region;

            grid[curr.y][curr.x].walls[curr_dir] = false;
            grid[next.y][next.x].walls[next_dir] = false;

            add_cell(next);
        }
    }
}

void Maze_grid::mazegen_kruskal(const sf::Vector2u & start, const int region)
{
    if(grid[start.y][start.x].visited)
        return;

    // floodfill to get all cells connected, add walls to list, cells to disjoint set, mark cell visited
    std::vector<Wall> walls;
    std::vector<sf::Vector2u> cells;
    std::vector<sf::Vector2u> to_add = {start};

    while(to_add.size() > 0)
    {
        sf::Vector2u curr = to_add.back();
        to_add.pop_back();

        if(grid[curr.y][curr.x].visited)
            continue;

        grid[curr.y][curr.x].visited = true;
        grid[curr.y][curr.x].region = region;
        cells.push_back(curr);

        for(int i = 0; i < 4; ++i)
        {
            switch(i)
            {
            case UP:
                if(curr.y > 0 && !grid[curr.y - 1][curr.x].visited)
                {
                    sf::Vector2u next(curr.x, curr.y - 1);
                    walls.push_back({curr, next, UP, DOWN});
                    to_add.push_back(next);
                }
                break;
            case DOWN:
                if(curr.y < grid.size() - 1 && !grid[curr.y + 1][curr.x].visited)
                {
                    sf::Vector2u next(curr.x, curr.y + 1);
                    walls.push_back({curr, next, DOWN, UP});
                    to_add.push_back(next);
                }
                break;
            case LEFT:
                if(curr.x > 0 && !grid[curr.y][curr.x - 1].visited)
                {
                    sf::Vector2u next(curr.x - 1, curr.y);
                    walls.push_back({curr, next, LEFT, RIGHT});
                    to_add.push_back(next);
                }
                break;
            case RIGHT:
                if(curr.x < grid[curr.y].size() - 1 && !grid[curr.y][curr.x + 1].visited)
                {
                    sf::Vector2u next(curr.x + 1, curr.y);
                    walls.push_back({curr, next, RIGHT, LEFT});
                    to_add.push_back(next);
                }
                break;
            }
        }
    }

    Disjoint_set<sf::Vector2u> cell_set(cells);

    // shuffle walls
    std::shuffle(walls.begin(), walls.end(), prng);

    // for wall in walls, and while # sets > 1
    size_t num_sets = cells.size();
    for(const auto & wall: walls)
    {
        sf::Vector2u set_1 = cell_set.find_rep(wall.cell_1);
        sf::Vector2u set_2 = cell_set.find_rep(wall.cell_2);

        if(set_1 != set_2)
        {
            cell_set.union_reps(set_1, set_2);

            // destroy walls
            grid[wall.cell_1.y][wall.cell_1.x].walls[wall.dir_1] = false;
            grid[wall.cell_2.y][wall.cell_2.x].walls[wall.dir_2] = false;

            if(--num_sets == 1)
                break;
        }
    }
}

class Maze
{
public:
    Maze(sf::RenderWindow & win, const sf::Vector2u & grid_size);
    void init();
    void draw();
    void set_grid_size(const sf::Vector2u & grid_size);
    sf::Vector2u get_grid_size() const;
private:
    Maze_grid _grid;
    sf::RenderWindow & _win;
    sf::VertexArray _lines;
};

Maze::Maze(sf::RenderWindow & win, const sf::Vector2u & grid_size):
    _grid(grid_size),
    _win(win),
    _lines(sf::Lines)
{
}

void Maze::init()
{
    _grid.init();

    sf::Vector2u grid_size(_grid.grid[0].size(), _grid.grid.size());
    sf::Vector2f cell_scale((float)_win.getSize().x / (float)grid_size.x, (float)_win.getSize().y / (float)grid_size.y);

    // draw border
    _lines.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color::Black));
    _lines.append(sf::Vertex(sf::Vector2f((float)_win.getSize().x, 0.0f), sf::Color::Black));

    _lines.append(sf::Vertex(sf::Vector2f((float)_win.getSize().x, 0.0f), sf::Color::Black));
    _lines.append(sf::Vertex(sf::Vector2f((float)_win.getSize().x, (float)_win.getSize().y), sf::Color::Black));

    _lines.append(sf::Vertex(sf::Vector2f((float)_win.getSize().x, (float)_win.getSize().y), sf::Color::Black));
    _lines.append(sf::Vertex(sf::Vector2f(0.0f, (float)_win.getSize().y), sf::Color::Black));

    _lines.append(sf::Vertex(sf::Vector2f(0.0f, (float)_win.getSize().y), sf::Color::Black));
    _lines.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color::Black));

    // draw maze cells
    for(size_t row = 0; row < grid_size.y; ++row)
    {
        for(size_t col = 0; col < grid_size.x; ++col)
        {
            sf::Vector2f ul(cell_scale.x * (float)col, cell_scale.y * (float)row);

            if(_grid.grid[row][col].walls[UP])
            {
                _lines.append(sf::Vertex(ul, sf::Color::Black));
                _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y), sf::Color::Black));
            }
            // if(_grid.grid[row][col].walls[DOWN])
            // {
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x, ul.y + cell_scale.y), sf::Color::Black));
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y + cell_scale.y), sf::Color::Black));
            // }
            if(_grid.grid[row][col].walls[LEFT])
            {
                _lines.append(sf::Vertex(ul, sf::Color::Black));
                _lines.append(sf::Vertex(sf::Vector2f(ul.x, ul.y + cell_scale.y), sf::Color::Black));
            }
            // if(_grid.grid[row][col].walls[RIGHT])
            // {
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y), sf::Color::Black));
            //     _lines.append(sf::Vertex(sf::Vector2f(ul.x + cell_scale.x, ul.y + cell_scale.y), sf::Color::Black));
            // }
        }
    }
}

void Maze::draw()
{
    _win.draw(_lines);
}

int main(int argc, char * argv[])
{
    prng.seed(rng());
    sf::RenderWindow win(sf::VideoMode(800, 600), "mazegen", sf::Style::Default);

    Maze maze(win, sf::Vector2u(32, 32));
    maze.init();

    while(win.isOpen())
    {
        win.clear(sf::Color(255, 255, 255, 255));
        maze.draw();
        win.display();
        sf::Event ev;

        if(win.waitEvent(ev))
        {
            switch(ev.type)
            {
                case sf::Event::Closed:
                    win.close();
                    break;
                case sf::Event::Resized:
                    break;
                default:
                    break;
            }
        }
    }

    return EXIT_SUCCESS;
}
