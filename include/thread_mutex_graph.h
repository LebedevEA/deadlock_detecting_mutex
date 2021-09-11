#pragma once

#include <thread>
#include <optional>
#include <unordered_map>

#include "deadlock_detecting_mutex.h"

class thread_mutex_graph {
public:
    static thread_mutex_graph& get();
    void wait(const deadlock_detecting_mutex& mutex);
    void stop_waiting();
    std::optional<std::thread::id> waits_for_mutex_held_by(std::thread::id id);

private:
    thread_mutex_graph() = default;

    std::mutex graph_mutex_;
    std::unordered_map<std::thread::id, const deadlock_detecting_mutex&> tid_to_waiting_{};
};
