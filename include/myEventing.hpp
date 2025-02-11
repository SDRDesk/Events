#pragma once

#include <cassert>
#include <csignal>
#include <cstdio>
#include <functional>
#include <deque>
#include <unordered_map>
#include <vector>
#include <thread>
#include <string>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <atomic>
#include <shared_mutex>
#include <chrono>

namespace Eventing {

std::atomic<int> event1Counter(0);
std::atomic<int> event2Counter(0);
std::atomic<int> event3Counter(0);
std::atomic<int> event4Counter(0);

class SingleThreaded {
    public:
    void lock() {}
    void unlock() {}
};

class MultiThreaded {
    public:
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    private:
    std::recursive_mutex mutex;
};

template <typename ThreadingPolicy, typename... Args>
class EventDispatcher {
    public:
    using Event = std::function<void(Args...)>;
    using IdleCallback = std::function<void()>;

    EventDispatcher() : running(true) {}
    ~EventDispatcher() { stop(); }

    template <typename EventType, typename... EventArgs>
    void postEvent(EventArgs&&... args) {
        std::lock_guard<ThreadingPolicy> lock(
            threadingPolicy);
        auto it = eventMap.find(
            std::type_index(typeid(EventType)));
        if (it != eventMap.end()) {
            for (const auto& event : it->second) {
                eventQueue.emplace_back(
                    [=,
                        ... args = std::forward<EventArgs>(
                            args)]() { event(args...); });
            }
        } else {
            throw std::runtime_error(
                "Error: Event type not found");
        }
    }

    void stop() {
        std::lock_guard<ThreadingPolicy> lock(
            threadingPolicy);
        running = false;
    }

    bool empty() const {
        std::lock_guard<ThreadingPolicy> lock(
            threadingPolicy);
        return eventQueue.empty();
    }

    void dispatchLoop(IdleCallback idleCallback) {
        while (running) {
            std::function<void()> event;
            {
                std::lock_guard<ThreadingPolicy> lock(
                    threadingPolicy);
                if (eventQueue.empty()) {
                    idleCallback();
                    continue;
                }
                event = std::move(eventQueue.front());
                eventQueue.pop_front();
            }
            event();
        }
    }

    template <typename EventType>
    void registerEvent(Event event) {
        std::lock_guard<ThreadingPolicy> lock(
            threadingPolicy);
        eventMap[std::type_index(typeid(EventType))]
            .emplace_back(std::move(event));
    }

    template <typename T, typename EventType,
        typename... MemberArgs>
    void registerEvent(T* instance,
        int (T::*memberFunc)(MemberArgs...) const) {
        registerEvent<EventType>(
            [instance, memberFunc](MemberArgs... args) {
                return (instance->*memberFunc)(args...);
            });
    }

    template <typename T, typename EventType,
        typename... MemberArgs>
    void registerEvent(
        T* instance, int (T::*memberFunc)(MemberArgs...)) {
        registerEvent<EventType>(
            [instance, memberFunc](MemberArgs... args) {
                return (instance->*memberFunc)(args...);
            });
    }

    private:
    std::deque<std::function<void()>> eventQueue;
    std::atomic<bool> running;
    std::unordered_map<std::type_index, std::vector<Event>>
        eventMap;
    mutable ThreadingPolicy threadingPolicy;
};
} // namespace Eventing
