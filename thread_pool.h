#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <functional>
#include <queue>
#include <mutex>
#include <atomic>
#include <future>
#include <condition_variable>
#include "threads_guard.h"

class thread_pool {

private:
    using Task = std::function<void(void)>;

    std::queue<Task> task_queue;
    std::vector<std::shared_ptr<std::thread>> pool;
    threads_guard tg;
    std::mutex task_mut;
    std::condition_variable task_cond;

    const std::atomic_int max_thread_num = std::thread::hardware_concurrency();
    std::atomic_int thread_num;
    std::atomic_int idle_thread_num;
    std::atomic_int core_thread_num; 
    std::atomic_bool done;

    void work_thread() {
        while(!done) {
            Task task;
            {
                std::unique_lock<std::mutex> lk(task_mut);
                if(thread_num > core_thread_num && task_queue.empty()) {
                    --thread_num;
                    --idle_thread_num;
                    lk.unlock();
                    break;
                }
                task_cond.wait(lk, [this]{return !task_queue.empty();});
                task = std::move(task_queue.front());
                task_queue.pop();
                --idle_thread_num;
                task();
                ++idle_thread_num;
            }
        }
    }

public:
    thread_pool(int i): done(false), tg(pool) {
        thread_num = i;
        core_thread_num = i;
        idle_thread_num = i;
        if(thread_num <= 0) {
            thread_num = max_thread_num / 2;
            core_thread_num = max_thread_num / 2;
            idle_thread_num = max_thread_num / 2;
        }


        try {
            for(unsigned index = 0; index < thread_num; ++index) {
                pool.emplace_back(std::make_shared<std::thread>(std::thread(&thread_pool::work_thread, this)));
            }
        } catch(...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
    }

    template<typename FunctionType, class... Args>
    std::future<typename std::result_of<FunctionType(Args...)>::type>
    submit (FunctionType&& f, Args&&... args) {
       typedef typename std::result_of<FunctionType(Args...)>::type return_type;

    //    std::packaged_task<result_type()> task(std::move(f));
       auto task = std::make_shared<std::packaged_task<return_type()>>(
           std::bind(std::forward<FunctionType>(f), std::forward<Args>(args)...)
       );

       std::future<return_type> res(task->get_future());
       {
           std::lock_guard<std::mutex> lk(task_mut);
           task_queue.emplace([task]{(*task)();});
       }
       task_cond.notify_one();
       return res;
    }

    int size() {
        std::lock_guard<std::mutex> lk(task_mut);
        return thread_num;
    }

    int idle_size() {
        std::lock_guard<std::mutex> lk(task_mut);
        return idle_thread_num;
    }

    int core_size() {
        std::lock_guard<std::mutex> lk(task_mut);
        return core_thread_num;
    }

    bool addWorker(int n, bool isCore) {
        // if(idle_thread_num >= thread_num / 2)
        //     return false;

        try {
            for(unsigned index = 0; index < thread_num; ++index) {
                pool.emplace_back(std::make_shared<std::thread>(std::thread(&thread_pool::work_thread, this)));
                ++thread_num;
                if(isCore)
                    ++core_thread_num;
            }
        } catch(...) {
            done = true;
            throw;
        }
    } 
};

#endif /* THREAD_POOL_H */