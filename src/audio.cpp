// audio.cpp
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

#include "audio.hpp"

#include "entity.hpp"
#include "logger.hpp"

Audio::Audio(const glm::vec3 & pos): _pos(pos.x, pos.y, pos.z)
{
}

std::shared_ptr<Audio> Audio::create(const glm::vec3 & pos)
{
    return std::make_shared<Audio>(pos);
}

void Audio::set_pos(const glm::vec3 & pos)
{
    _pos.x = pos.x;
    _pos.y = pos.y;
    _pos.z = pos.z;

    for(auto & sound: _sound_players)
    {
        sound->setPosition(_pos);
    }

    _music_player.setPosition(_pos);
}

std::shared_ptr<sf::Sound> Audio::play_sound(const std::string & filename, const float volume,
    const bool loop)
{
    std::shared_ptr<sf::Sound> sound = std::make_shared<sf::Sound>(Jukebox::get_sound(filename));
    _sound_players.push_back(sound);

    sound->setPosition(_pos);
    sound->setVolume(volume);
    sound->setLoop(loop);
    sound->play();

    return sound;
}

sf::Music & Audio::play_music(const std::string & filename, const float volume,
    const bool loop)
{
    _music_player.openFromFile(filename);

    _music_player.setPosition(_pos);
    _music_player.setVolume(volume);
    _music_player.setLoop(loop);
    _music_player.play();

    return _music_player;
}

void Audio::silence_all()
{
    for(auto & sound: _sound_players)
        sound->stop();
    _sound_players.clear();

    _music_player.stop();
}

void Audio::update(const Entity & ent)
{
    for(auto it = _sound_players.begin(); it != _sound_players.end(); ++it)
    {
        if((*it)->getStatus() == sf::Sound::Stopped)
        {
            it = _sound_players.erase(it);
        }
    }
    set_pos(ent.pos());
}

std::mutex Jukebox::_lock;
std::unordered_map<std::string, sf::SoundBuffer> Jukebox::_store;

void Jukebox::preload_sound(const std::string & filename)
{
    _lock.lock();
    if(_store.count(filename) > 0)
        return;
    Logger_locator::get()(Logger::DBG, "Loading sound: " + filename);
    if(!_store[filename].loadFromFile(filename))
    {
        Logger_locator::get()(Logger::ERROR, std::string("Error reading audio file: ") + filename);
        throw std::ios_base::failure(std::string("Error reading audio file: ") + filename);
    }
    _lock.unlock();
}

void Jukebox::unload_sound(const std::string & filename)
{
    _lock.lock();
    auto it = _store.find(filename);
    if(it != _store.end())
    {
        _store.erase(it);
        Logger_locator::get()(Logger::DBG, "Unloading sound: " + filename);
    }
    _lock.unlock();
}

sf::Sound Jukebox::get_sound(const std::string & filename)
{
    _lock.lock();
    sf::Sound sound(_store.at(filename));
    _lock.unlock();
    return sound;
}
