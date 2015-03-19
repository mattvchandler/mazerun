// message.hpp
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

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <sigc++/sigc++.h>

#include "logger.hpp"

class Message
{
private:
    template<typename T>
    class Typed_packet;

public:
    class Packet
    {
    public:
        virtual ~Packet() = default;
    };

    ~Message();
    void rm_event(const std::string & event);

    sigc::connection add_callback(const std::string & event,
        const sigc::slot<void, const Packet &> & callback);

    sigc::connection add_callback_empty(const std::string & event,
        const sigc::slot<void> & callback);

    template<typename T>
    void queue_event(const std::string & event, T && t);

    template<typename T>
    void queue_event(const std::string & event, const T & t);

    void queue_event_empty(const std::string & event);

    bool queue_empty();

    void process_events();

    template<typename T>
    static const T & get_packet(const Packet & pkt);

private:
    std::unordered_map<std::string, sigc::signal<void, const Packet &>> _signals;
    std::vector<std::pair<std::string, std::unique_ptr<Packet>>> _queue;
    std::mutex _lock;
};

template<typename T>
class Message::Typed_packet: public Message::Packet
{
public:
    Typed_packet() = delete;
    Typed_packet(const T & t): msg(t)
    {
    }
    Typed_packet(T && t): msg(std::move(t))
    {
    }
    const T & get() const
    {
        return msg;
    }
    T msg;
};

template<typename T>
void Message::queue_event(const std::string & event, T && t)
{
    _lock.lock();

    _queue.push_back(std::make_pair(event, std::unique_ptr<Packet>(new Typed_packet<T>(std::forward<T>(t)))));

    _lock.unlock();
}

template<typename T>
void Message::queue_event(const std::string & event, const T & t)
{
    _lock.lock();

    _queue.push_back(std::make_pair(event, std::unique_ptr<Packet>(new Typed_packet<T>(t))));

    _lock.unlock();
}

template<typename T>
const T & Message::get_packet(const Packet & pkt)
{
    const T & data = dynamic_cast<const Typed_packet<T> &>(pkt).get();
    return data;
}

class Message_locator
{
    public:
        Message_locator() = delete;
        ~Message_locator() = delete;

        static void init(std::shared_ptr<Message> msg = std::make_shared<Message>());
        static Message & get();

    private:
        static std::shared_ptr<Message> _msg;
};

#endif // MESSAGE_HPP
