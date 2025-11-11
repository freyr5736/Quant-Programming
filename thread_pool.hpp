#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class thread_pool {
  public:
    thread_pool(size_t total_threads) {
        for (size_t i = 0; i < total_threads; ++i) {
            all_threads.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(q_mutex);
                        cv_task.wait(
                            lock, [this]() { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) {
                            return; // exit thread
                        }
                        task = std::move(tasks.front()); // extract the task
                        tasks.pop(); // remove task from the queue
                    }
                    // track active task
                    active_tasks.fetch_add(1, std::memory_order_relaxed);
                    // if i donot use scopes, then it will run task with lock
                    // which can create deadlock, alternative is to unlock before
                    // running the task and lock after that but it creates overhead
                    task(); // execute the task

                    // mark task as done
                    active_tasks.fetch_sub(1, std::memory_order_relaxed);

                    // wake up any waiters
                    std::unique_lock<std::mutex> wait_lock(wait_mutex);
                    if (tasks.empty() && active_tasks.load() == 0)
                        cv_wait.notify_all();
                }
            });
        }
    }

    // template <class T> void enqueue(T &&task) {
    //     std::unique_lock<std::mutex> lock(q_mutex);
    //     tasks.emplace(std::forward<T>(task)); // forward<T> pushes value
    //     exactly as it is lock.unlock(); cv_task.notify_one(); // signalling
    //     that the task is available

    // }

    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)-> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        /*
        packages a function and its return value into something that can be:
        executed later, and
        whose result can be retrieved through a std::future.

        Component	                                             Meaning
        std::invoke_result<F, Args...>	                         Deduces the function’s return type
        std::bind	                                             Pre-binds arguments to make a zero-arg callable
        std::packaged_task<return_type()>	                     Wraps callable so it can produce a std::future
        std::make_shared	                                     Safely shares the task between threads
        task->get_future()	                                     Gives the caller a handle to wait for the result
        */
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(q_mutex);
            tasks.emplace([task]() { (*task)(); });
        }
        cv_task.notify_one();
        return res;
    }
    // often return too early before tasks are finished.
    // void pool_wait() {
    //     std::unique_lock<std::mutex> lock(q_mutex);
    //     cv_task.wait(lock, [this] { return tasks.empty(); });
    // }

    void wait_for_tasks() {
        std::unique_lock<std::mutex> lock(wait_mutex);
        cv_wait.wait(lock, [this]() {
            std::unique_lock<std::mutex> q_lock(q_mutex);
            return tasks.empty() && active_tasks.load() == 0;
        });
    }
    

    ~thread_pool() {
        std::unique_lock<std::mutex> lock(q_mutex);
        stop = true;
        lock.unlock();
        cv_task
            .notify_all(); // wake up all threads to finish their tasks and exit
        for (auto &thread : all_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

  private:
    std::vector<std::thread> all_threads;
    std::queue<std::function<void()>>
        tasks; // can send anything inside the queue that is what function<void()> is
    std::mutex q_mutex;
    std::condition_variable cv_task;
    std::atomic<size_t> active_tasks{0};
    std::condition_variable cv_wait;
    std::mutex wait_mutex;
    bool stop{false};
};