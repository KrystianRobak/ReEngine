#pragma once


#include "ReEngineExport.h"
#include "Logger.h"

#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <queue>
#include <condition_variable>
#include <atomic>


class ENGINE_API ThreadPool {
public:
	ThreadPool(size_t threads = std::thread::hardware_concurrency());
	~ThreadPool();

	template<class F, class... Args>
	auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
		using return_type = std::invoke_result_t<F, Args...>;
		auto taskPtr = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		std::future<return_type> res = taskPtr->get_future();
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			if (stop.load()) throw std::runtime_error("submit on stopped ThreadPool");
			tasks.emplace([taskPtr]() {
				(*taskPtr)(); 
				
				LOGF_INFO("Task executed in thread %d", std::this_thread::get_id());
				});
		}
		condition.notify_one();
		return res;
	}


	void shutdown();
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;


	std::mutex queueMutex;
	std::condition_variable condition;
	std::atomic<bool> stop{ false };
};