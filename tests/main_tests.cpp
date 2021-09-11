#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <deadlock_detecting_mutex.h>
#include "doctest.h"

TEST_CASE("simple | single mutex | single thread") {
    deadlock_detecting_mutex mutex;
    mutex.lock();
    CHECK_THROWS_WITH_AS(mutex.lock(), "Deadlock detected!", std::logic_error);
}

TEST_CASE("simple | single mutex | two threads") {
    deadlock_detecting_mutex mutex;
    std::thread fst([&](){
        mutex.lock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK_THROWS_WITH_AS(mutex.lock(), "Deadlock detected!", std::logic_error);
    });
    std::thread snd([&](){
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        mutex.try_lock();
    });
    fst.join();
    snd.join();
}

TEST_CASE("basic | two mutexes | two threads") {
    deadlock_detecting_mutex mutex1;
    deadlock_detecting_mutex mutex2;
    std::atomic_flag did_throw;
    std::thread fst([&](){
        mutex1.lock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        try {
            mutex2.lock();
        } catch (std::logic_error& e) {
            if ("Deadlock detected!" == std::string(e.what()))
                did_throw.test_and_set();
        }
    });
    std::thread snd([&](){
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        mutex2.lock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        try {
            mutex1.lock();
        } catch (std::logic_error& e) {
            if ("Deadlock detected!" == std::string(e.what()))
                did_throw.test_and_set();
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CHECK(did_throw.test_and_set());

    mutex1.unlock();
    mutex2.unlock();

    fst.join();
    snd.join();
}

void run_basic(int n) {
    std::vector<deadlock_detecting_mutex> mutexes(n);
    std::vector<std::thread> threads;
    std::atomic_flag did_throw;
    threads.reserve(n);
    for (int i = 0; i < n; i++) {
        threads.emplace_back([n, i, &mutexes, &did_throw](){
            std::this_thread::sleep_for(std::chrono::milliseconds(i * 50));
            mutexes[i].lock();
            std::this_thread::sleep_for(std::chrono::milliseconds(n * 50));
            try {
                mutexes[(n + i - 1) % n].lock();
            } catch (std::logic_error& e) {
                if ("Deadlock detected!" == std::string(e.what()))
                    did_throw.test_and_set();
            }
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(n * 150));
    CHECK(did_throw.test_and_set());

    std::for_each(mutexes.begin(), mutexes.end(), [](auto& m){ m.unlock(); });
    std::for_each(threads.begin(), threads.end(), [](auto& t){ t.join(); });
}

TEST_CASE("basic | n mutexes | n threads | many times") {
    for (int i = 0; i < 3; i++) { // 3 only 'cause more takes way too long...
        run_basic(i + 10);
    }
}

TEST_CASE("many lock-unlock-try-lock | single mutex | single thread") {
    deadlock_detecting_mutex mutex;
    mutex.lock();
    mutex.unlock();
    mutex.lock();
    mutex.unlock();
    mutex.lock();
    mutex.try_lock();
    mutex.unlock();
    mutex.try_lock();
    mutex.unlock();
    mutex.lock();
    mutex.try_lock();
    CHECK_THROWS_WITH_AS(mutex.lock(), "Deadlock detected!", std::logic_error);
}

TEST_CASE("simple #1 | two mutexes | single thread") {
    deadlock_detecting_mutex mutex1;
    mutex1.lock();
    CHECK_THROWS_WITH_AS(mutex1.lock(), "Deadlock detected!", std::logic_error);
    deadlock_detecting_mutex mutex2;
    mutex2.lock();
    CHECK_THROWS_WITH_AS(mutex2.lock(), "Deadlock detected!", std::logic_error);
}

TEST_CASE("simple #2 | two mutexes | single thread") {
    deadlock_detecting_mutex mutex1;
    deadlock_detecting_mutex mutex2;
    mutex1.lock();
    mutex2.lock();
    CHECK_THROWS_WITH_AS(mutex1.lock(), "Deadlock detected!", std::logic_error);
    CHECK_THROWS_WITH_AS(mutex2.lock(), "Deadlock detected!", std::logic_error);
}

TEST_CASE("many lock-unlock-try-lock | two mutexes | single thread") {
    deadlock_detecting_mutex mutex1;
    deadlock_detecting_mutex mutex2;
    mutex1.lock();
    mutex2.try_lock();
    mutex1.unlock();
    mutex1.lock();
    mutex2.try_lock();
    mutex2.unlock();
    mutex1.unlock();
    mutex2.lock();
    mutex1.lock();
    mutex1.try_lock();
    mutex2.unlock();
    mutex1.unlock();
    mutex2.lock();
    mutex1.try_lock();
    mutex1.unlock();
    mutex2.unlock();
    mutex1.lock();
    mutex1.try_lock();
    mutex2.lock();
    CHECK_THROWS_WITH_AS(mutex1.lock(), "Deadlock detected!", std::logic_error);
    CHECK_THROWS_WITH_AS(mutex2.lock(), "Deadlock detected!", std::logic_error);
}
