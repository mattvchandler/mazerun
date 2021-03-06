// audio.hpp
// audio system

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

#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include <SFML/Audio.hpp>

#include "components/component.hpp"

class Audio: public Component
{
public:
    Audio(const glm::vec3 & pos);
    virtual ~Audio() = default;
    virtual void set_pos(const glm::vec3 & pos);
    virtual sf::Sound * play_sound(const std::string & filename, const float volume = 100.0f,
        const bool loop = false);
    virtual sf::Music & play_music(const std::string & filename, const float volume = 100.0f,
        const bool loop = false);
    virtual void silence_all();
    virtual void update(const Entity & ent);

private:
    sf::Vector3f _pos;
    std::list<sf::Sound> _sound_players;
    sf::Music _music_player;
};

class Jukebox_base
{
public:
    virtual ~Jukebox_base() = default;
    virtual void preload_sound(const std::string & filename);
    virtual void unload_sound(const std::string & filename);
    virtual sf::Sound get_sound(const std::string & filename);
};

class Jukebox: public Jukebox_base
{
public:
    ~Jukebox();
    void preload_sound(const std::string & filename);
    void unload_sound(const std::string & filename);
    sf::Sound get_sound(const std::string & filename);

private:
    std::mutex _lock;
    std::unordered_map<std::string, sf::SoundBuffer> _store;
};

class Jukebox_locator
{
public:
    Jukebox_locator() = delete;
    ~Jukebox_locator() = delete;
    static void init(Jukebox_base * jukebox);
    static Jukebox_base & get();

private:
    static Jukebox_base _default_jukebox;
    static Jukebox_base * _jukebox;
};

#endif // AUDIO_HPP
