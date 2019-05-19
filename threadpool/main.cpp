#include <iostream>
#include <thread>
#include <future>

#include "thread_pool.h"

void fun1(int slp)
{
	printf("  hello, fun1 !  %d\n" ,std::this_thread::get_id());
	if (slp>0) {
		printf(" ======= fun1 sleep %d  =========  %d\n",slp, std::this_thread::get_id());
		std::this_thread::sleep_for(std::chrono::milliseconds(slp));
		//Sleep(slp );
	}
}

struct gfun {
	int operator()(int n) {
		printf("%d  hello, gfun !  %d\n" ,n, std::this_thread::get_id() );
		return 42;
	}
};

int main () {
    thread_pool *pool = new thread_pool(5);
    std::future<void> ff = pool->submit(fun1, 1); 

    std::future<int> gf = pool->submit(gfun(), 0);
    std::cout<< gf.get() << std::endl;
    return 0;
}