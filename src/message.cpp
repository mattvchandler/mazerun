// message.cpp
// message passing system

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

#include "message.hpp"

std::unordered_map<std::string, sigc::signal<void, const Message::Packet &>> Message::_signals;
std::vector<std::pair<std::string, std::unique_ptr<Message::Packet>>> Message::_queue;
std::mutex Message::_lock;

void Message::add_event(const std::string & event)
{
    _lock.lock();
    if(_signals.count(event) > 0)
    {
        return;
    }

    _signals[event];
    _lock.unlock();
}

void Message::rm_event(const std::string & event)
{
    _lock.lock();
    auto it = _signals.find(event);
    if(it != _signals.end())
        _signals.erase(it);
    _lock.unlock();
}

sigc::connection Message::add_callback(const std::string & event,
    const sigc::slot<void, const Packet &> & callback)
{
    _lock.lock();
    sigc::connection conn = _signals.at(event).connect(callback);
    _lock.unlock();
    return conn;
}

sigc::connection Message::add_callback_empty(const std::string & event,
    const sigc::slot<void> & callback)
{
    _lock.lock();
    sigc::connection conn = _signals.at(event).connect(sigc::hide(callback));
    _lock.unlock();
    return conn;
}

void Message::queue_event_empty(const std::string & event)
{
    _lock.lock();
    _queue.push_back(std::make_pair(event, std::unique_ptr<Packet>()));
    _lock.unlock();
}

void Message::process_events()
{
    _lock.lock();
    for(const auto & event: _queue)
    {
        _signals.at(event.first)(*event.second);
    }
    _queue.clear();
    _lock.unlock();
}

bool Message::queue_empty()
{
    _lock.lock();
    bool empty = _queue.empty();
    _lock.unlock();
    return empty;
}
