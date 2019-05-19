#include <thread>
#include <mutex>
class threads_guard {
private:
    std::vector<std::shared_ptr<std::thread>> _threads;

public:
    threads_guard(std::vector<std::shared_ptr<std::thread>> &threads)
        :_threads(threads) {}

    ~threads_guard() {
        for(auto t = _threads.begin(); t != _threads.end(); ++t) {
            if((*t)->joinable() == true) {
                (*t)->join();
            }
        }
    }

private:
    threads_guard(threads_guard&& tg) = delete;
    threads_guard& operator=(threads_guard&& tg) = delete;
    threads_guard& operator=(const threads_guard&) = delete;
    threads_guard(const threads_guard&) = delete;
};