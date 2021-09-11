#include "thread_mutex_graph.h"
#include "deadlock_detecting_mutex.h"

thread_mutex_graph& thread_mutex_graph::get() {
    static thread_mutex_graph graph;
    return graph;
}

void thread_mutex_graph::wait(const deadlock_detecting_mutex &mutex) {
    const std::lock_guard<std::mutex> lock(graph_mutex_);
    tid_to_waiting_.insert({ std::this_thread::get_id(), mutex });
}

void thread_mutex_graph::stop_waiting() {
    const std::lock_guard<std::mutex> lock(graph_mutex_);
    tid_to_waiting_.erase(std::this_thread::get_id());
}

std::optional<std::thread::id> thread_mutex_graph::waits_for_mutex_held_by(const std::thread::id id) {
    const std::lock_guard<std::mutex> lock(graph_mutex_);
    if (tid_to_waiting_.count(id) == 0) return {};
    const deadlock_detecting_mutex& mutex = tid_to_waiting_.at(id);
    return mutex.held_by_;
}
