#include <thread>
#include "deadlock_detecting_mutex.h"

#include "thread_mutex_graph.h"

bool deadlock_detecting_mutex::operator==(const deadlock_detecting_mutex& that) const {
    return this == &that;
}

void deadlock_detecting_mutex::lock() {
    wait();
    if (!can_acquire()) throw std::logic_error("Deadlock detected!");
    base_.lock();
    acquire();
}

bool deadlock_detecting_mutex::try_lock() {
    if (base_.try_lock()) {
        acquire();
        return true;
    }
    return false;
}

void deadlock_detecting_mutex::unlock() {
    release();
    base_.unlock();
}

void deadlock_detecting_mutex::acquire() {
    held_by_ = std::this_thread::get_id();
    thread_mutex_graph::get().stop_waiting();
}

void deadlock_detecting_mutex::release() {
    held_by_.reset();
}

bool deadlock_detecting_mutex::can_acquire() const {
    auto thread = held_by_;
    while (thread.has_value()) {
        if (thread == std::this_thread::get_id()) return false;
        thread = thread_mutex_graph::get().waits_for_mutex_held_by(thread.value());
    }
    return true;
}

void deadlock_detecting_mutex::wait() const {
    thread_mutex_graph::get().wait(*this);
}
