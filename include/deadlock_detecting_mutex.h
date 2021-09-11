#pragma once

#include <mutex>
#include <optional>
#include <unordered_map>
#include <thread>

class deadlock_detecting_mutex {
public:
    deadlock_detecting_mutex() = default;

    bool operator==(const deadlock_detecting_mutex& that) const;

    void lock();
    bool try_lock();
    void unlock();

private:
    void acquire();
    void release();

    [[nodiscard]] bool can_acquire() const;
    void wait() const;

    std::optional<std::thread::id> held_by_{};
    std::mutex base_{};

    friend class thread_mutex_graph;
};